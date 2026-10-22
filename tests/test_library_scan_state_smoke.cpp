// SPDX-License-Identifier: AGPL-3.0-or-later
// Regression coverage for the library worker lifecycle and scan generations.

#include "test_framework.h"
#include "LibraryScanState.h"
#include "LibraryFileSystem.h"

#include <atomic>
#include <climits>
#include <thread>
#include <vector>

using EnvyLibraryScan::Generation;
using EnvyLibraryScan::Phase;

namespace
{
// Cancellation modelled exactly like CThreadImpl: a manual-reset event polled
// with a zero timeout.
struct CancelEvent
{
	HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	~CancelEvent() { CloseHandle(hEvent); }
	bool IsEnabled() const { return WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT; }
	void Cancel() { SetEvent(hEvent); }
};

// Adapts three callables to the worker interface EnvyLibraryScan::RunWorker() expects.
template<class E, class L, class P>
struct FnWorker
{
	E isEnabled;
	L completeLazy;
	P runPass;
	bool IsThreadEnabled() { return isEnabled(); }
	void CompleteLazyLoading() { completeLazy(); }
	void RunMaintenancePass() { runPass(); }
};

template<class E, class L, class P>
void RunCallables(E isEnabled, L completeLazy, P runPass)
{
	FnWorker<E, L, P> worker{ isEnabled, completeLazy, runPass };
	EnvyLibraryScan::RunWorker(worker);
}

bool WaitFor(const std::atomic<int>& value, int target, DWORD timeoutMs = 5000)
{
	const ULONGLONG deadline = GetTickCount64() + timeoutMs;
	while (value.load() < target)
	{
		if (GetTickCount64() > deadline)
			return false;
		Sleep(1);
	}
	return true;
}
} // namespace

static bool test_scan_generation_lifecycle()
{
	Generation gen;
	LONG nBegin = 0;
	if (gen.Begin(nBegin))
		return false; // Nothing requested yet

	const LONG nTarget = gen.Request();
	if (gen.GetPhase(nTarget) != Phase::Pending)
		return false;
	if (!gen.Begin(nBegin) || nBegin != nTarget || gen.GetPhase(nTarget) != Phase::Scanning)
		return false;
	gen.Complete(nBegin);
	return gen.GetPhase(nTarget) == Phase::Completed && !gen.Begin(nBegin);
}

// Several UI requests before the worker runs are satisfied by one scan.
static bool test_scan_generation_coalesces_pending_requests()
{
	Generation gen;
	const LONG a = gen.Request();
	const LONG b = gen.Request();
	const LONG c = gen.Request();
	LONG n = 0;
	if (!gen.Begin(n) || n != c)
		return false;
	gen.Complete(n);
	return gen.GetPhase(a) == Phase::Completed && gen.GetPhase(b) == Phase::Completed &&
	       gen.GetPhase(c) == Phase::Completed && !gen.Begin(n);
}

// A request made while a scan runs is not satisfied by that scan.
static bool test_scan_generation_request_during_scan_needs_next_pass()
{
	Generation gen;
	gen.Request();
	LONG first = 0;
	if (!gen.Begin(first))
		return false;
	const LONG late = gen.Request();
	gen.Complete(first);
	if (gen.GetPhase(late) != Phase::Pending)
		return false;
	LONG second = 0;
	if (!gen.Begin(second) || second != late)
		return false;
	gen.Complete(second);
	return gen.GetPhase(late) == Phase::Completed;
}

// A waiter joins an unstarted pending request instead of stacking a second
// full pass: Update( true ) then the dialog's RequestScan() must scan once.
static bool test_scan_generation_join_unstarted_request()
{
	Generation gen;
	if (gen.RequestOrJoin() != 1)
		return false; // Nothing pending: issue the first request
	const LONG joined = gen.RequestOrJoin();
	LONG n = 0;
	if (joined != 1 || !gen.Begin(n) || n != 1)
		return false;
	gen.Complete(n);
	return gen.GetPhase(joined) == Phase::Completed;
}

// A running scan cannot cover changes made after it started, so a waiter
// that arrives then still stacks its own request.
static bool test_scan_generation_join_running_scan_issues_next()
{
	Generation gen;
	const LONG first = gen.Request();
	LONG n = 0;
	if (!gen.Begin(n) || n != first)
		return false;
	if (gen.RequestOrJoin() != first + 1)
		return false; // Scan running: issue the next request
	gen.Complete(first);
	LONG second = 0;
	if (!gen.Begin(second) || second != first + 1)
		return false;
	gen.Complete(second);
	return gen.RequestOrJoin() == first + 2; // Nothing pending again
}

// A pass cut short (shutdown) never completes; the next pass retries it.
static bool test_scan_generation_interrupted_scan_stays_pending()
{
	Generation gen;
	const LONG nTarget = gen.Request();
	LONG n = 0;
	if (!gen.Begin(n))
		return false;
	// No Complete(): the pass was interrupted.
	if (gen.GetPhase(nTarget) == Phase::Completed)
		return false;
	LONG retry = 0;
	return gen.Begin(retry) && retry == nTarget;
}

static bool test_scan_generation_wraparound()
{
	return EnvyLibraryScan::Reached(LONG_MIN, LONG_MAX) && !EnvyLibraryScan::Reached(LONG_MAX, LONG_MIN) &&
	       EnvyLibraryScan::Reached(5, 5) && !EnvyLibraryScan::Reached(4, 5);
}

// GetTickCount() wraps every 49.7 days; the periodic-scan test must not stall.
static bool test_tick_elapsed_wraparound()
{
	return EnvyLibraryScan::Elapsed(0x00001000u, 0xFFFFF000u, 0x1000u) &&
	       !EnvyLibraryScan::Elapsed(0x00000100u, 0xFFFFFF00u, 0x1000u) &&
	       EnvyLibraryScan::Elapsed(10000u, 0u, 5000u) && !EnvyLibraryScan::Elapsed(4000u, 0u, 5000u);
}

// Requests from several threads all complete with a single worker looping.
static bool test_scan_generation_concurrent_requests()
{
	Generation gen;
	CancelEvent cancel;
	std::atomic<int> passes{ 0 };
	std::thread worker([&]()
	                   {
		while (cancel.IsEnabled())
		{
			LONG n = 0;
			if (gen.Begin(n))
			{
				Sleep(0);
				gen.Complete(n);
				++passes;
			}
			else
				Sleep(1);
		} });

	std::vector<LONG> targets[4];
	std::vector<std::thread> clients;
	for (int i = 0; i < 4; ++i)
		clients.emplace_back([&, i]()
		                     {
			for (int j = 0; j < 200; ++j)
				targets[i].push_back(gen.Request()); });
	for (auto& t : clients)
		t.join();

	bool ok = true;
	const ULONGLONG deadline = GetTickCount64() + 5000;
	for (auto& list : targets)
		for (LONG target : list)
			while (gen.GetPhase(target) != Phase::Completed)
			{
				if (GetTickCount64() > deadline)
				{
					ok = false;
					break;
				}
				Sleep(1);
			}
	cancel.Cancel();
	worker.join();
	return ok && passes.load() > 0 && passes.load() <= 800;
}

// Library with files: deferred loading runs first, then the same thread keeps
// running maintenance passes, and it stops on cancellation.
static bool test_worker_runs_lazy_then_maintenance()
{
	CancelEvent cancel;
	std::atomic<int> lazy{ 0 }, passes{ 0 }, passesBeforeLazy{ 0 };
	std::thread worker([&]()
	                   { RunCallables([&]()
		                              { return cancel.IsEnabled(); },
		                              [&]()
		                              {
			                              Sleep(20); // Deferred work takes time
			                              ++lazy;
		                              },
		                              [&]()
		                              {
			                              if (lazy.load() == 0)
				                              ++passesBeforeLazy;
			                              ++passes;
			                              Sleep(1);
		                              }); });
	const bool bRan = WaitFor(passes, 5);
	cancel.Cancel();
	worker.join();
	const int nAfterStop = passes.load();
	Sleep(10);
	return bRan && lazy.load() == 1 && passesBeforeLazy.load() == 0 && passes.load() == nAfterStop;
}

// Cancellation while deferred loading runs: no maintenance pass starts.
static bool test_worker_cancel_during_lazy_loading()
{
	CancelEvent cancel;
	HANDLE hRelease = CreateEventW(NULL, TRUE, FALSE, NULL);
	std::atomic<int> lazyEntered{ 0 }, passes{ 0 };
	std::thread worker([&]()
	                   { RunCallables([&]()
		                              { return cancel.IsEnabled(); },
		                              [&]()
		                              {
			                              ++lazyEntered;
			                              WaitForSingleObject(hRelease, 5000);
		                              },
		                              [&]()
		                              { ++passes; }); });
	const bool bEntered = WaitFor(lazyEntered, 1);
	cancel.Cancel();
	SetEvent(hRelease);
	worker.join();
	CloseHandle(hRelease);
	return bEntered && passes.load() == 0;
}

// Shutdown requested before the worker runs: it exits without touching anything.
static bool test_worker_cancelled_before_start()
{
	CancelEvent cancel;
	cancel.Cancel();
	int lazy = 0, passes = 0;
	RunCallables([&]()
	             { return cancel.IsEnabled(); }, [&]()
	             { ++lazy; }, [&]()
	             { ++passes; });
	return lazy == 0 && passes == 0;
}

// Model of the defect: CThreadImpl::BeginThread() returns early while its
// thread is alive, and CLibrary::Load() held the library lock while starting a
// lazy-only thread and then the maintenance thread. The second start is then
// always ignored and no maintenance pass ever runs. The single worker built on
// RunWorker() keeps running passes under the same conditions.
static bool test_single_worker_survives_ignored_second_start()
{
	struct ThreadSlot
	{
		std::thread thread;
		std::atomic<bool> alive{ false };
		bool Begin(std::function<void()> body)
		{
			if (alive.load())
				return true; // Same contract as CThreadImpl::BeginThread()
			if (thread.joinable())
				thread.join();
			alive = true;
			thread = std::thread([this, body]()
			                     {
				body();
				alive = false; });
			return true;
		}
		~ThreadSlot()
		{
			if (thread.joinable())
				thread.join();
		}
	};

	auto runScenario = [](bool bSingleWorker)
	{
		CancelEvent cancel;
		HANDLE hLibraryLock = CreateEventW(NULL, TRUE, FALSE, NULL); // Held by Load() until set
		std::atomic<int> passes{ 0 };
		auto lazy = [&]()
		{ WaitForSingleObject(hLibraryLock, 5000); };
		auto maintenance = [&]()
		{
			while (cancel.IsEnabled())
			{
				++passes;
				Sleep(1);
			}
		};
		{
			ThreadSlot slot;
			if (bSingleWorker)
			{
				slot.Begin([&]()
				           { RunCallables([&]()
					                      { return cancel.IsEnabled(); }, lazy, [&]()
					                      {
						++passes;
						Sleep(1); }); });
			}
			else
			{
				slot.Begin(lazy);        // "LibraryLazy"
				slot.Begin(maintenance); // "Library": ignored, the lazy thread is alive
			}
			SetEvent(hLibraryLock); // Load() returns and releases the lock
			WaitFor(passes, 5, 500);
			cancel.Cancel();
		}
		CloseHandle(hLibraryLock);
		return passes.load();
	};

	return runScenario(false) == 0 && runScenario(true) >= 5;
}


// Path relations decide which shares are nested; prefixes that only look
// like a parent ("C:\Media" vs "C:\MediaExtra") must stay unrelated.
static bool test_path_relation_nesting()
{
	using EnvyFileSystem::PathRelation;
	using EnvyFileSystem::RelatePaths;
	return RelatePaths(L"C:\\Media", L"c:\\media") == PathRelation::Same &&
	       RelatePaths(L"C:\\Media\\Films", L"C:\\Media") == PathRelation::Descendant &&
	       RelatePaths(L"C:\\Media", L"C:\\Media\\Films") == PathRelation::Ancestor &&
	       RelatePaths(L"C:\\MediaExtra", L"C:\\Media") == PathRelation::Unrelated &&
	       RelatePaths(L"C:\\Media", L"C:\\MediaExtra") == PathRelation::Unrelated &&
	       RelatePaths(L"D:\\Media", L"C:\\Media") == PathRelation::Unrelated &&
	       RelatePaths(L"", L"C:\\Media") == PathRelation::Unrelated &&
	       // Drive roots and deep nesting decide parent/child adoption.
	       RelatePaths(L"C:", L"C:\\Media") == PathRelation::Ancestor &&
	       RelatePaths(L"C:\\Media", L"C:") == PathRelation::Descendant &&
	       RelatePaths(L"C:\\Media\\Films\\2024", L"C:\\Media") == PathRelation::Descendant &&
	       RelatePaths(L"C:\\Media", L"C:\\Media\\Films\\2024") == PathRelation::Ancestor &&
	       !EnvyFileSystem::EnumerationComplete(ERROR_CANCELLED);
}

// A complete listing returns every entry except "." and "..".
static bool test_list_directory_complete()
{
	wchar_t temp[MAX_PATH];
	if (GetTempPathW(MAX_PATH, temp) == 0 || GetTempPathW(MAX_PATH, temp) >= MAX_PATH)
		return false;
	std::wstring root = std::wstring(temp) + L"EnvyListTest" + std::to_wstring(GetCurrentProcessId()) + L"_" +
	                    std::to_wstring(GetTickCount64());
	if (!CreateDirectoryW(root.c_str(), NULL))
		return false;
	HANDLE hFile = CreateFileW((root + L"\\a.bin").c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	CreateDirectoryW((root + L"\\sub").c_str(), NULL);

	std::vector<EnvyFileSystem::DirectoryEntry> entries;
	const DWORD nError = EnvyFileSystem::ListDirectory(root + L"\\*.*", entries, []()
	                                                   { return false; });

	DeleteFileW((root + L"\\a.bin").c_str());
	RemoveDirectoryW((root + L"\\sub").c_str());
	RemoveDirectoryW(root.c_str());

	return hFile != INVALID_HANDLE_VALUE && entries.size() == 2 && EnvyFileSystem::EnumerationComplete(nError);
}

// Cancellation stops the listing and reports it as cancelled, never as complete.
static bool test_list_directory_cancelled()
{
	wchar_t temp[MAX_PATH];
	if (GetTempPathW(MAX_PATH, temp) == 0 || GetTempPathW(MAX_PATH, temp) >= MAX_PATH)
		return false;
	std::wstring root = std::wstring(temp) + L"EnvyListCancel" + std::to_wstring(GetCurrentProcessId()) + L"_" +
	                    std::to_wstring(GetTickCount64());
	if (!CreateDirectoryW(root.c_str(), NULL))
		return false;
	for (int i = 0; i < 3; ++i)
		CreateDirectoryW((root + L"\\d" + std::to_wstring(i)).c_str(), NULL);

	std::vector<EnvyFileSystem::DirectoryEntry> entries;
	const DWORD nError = EnvyFileSystem::ListDirectory(root + L"\\*.*", entries, []()
	                                                   { return true; });

	for (int i = 0; i < 3; ++i)
		RemoveDirectoryW((root + L"\\d" + std::to_wstring(i)).c_str());
	RemoveDirectoryW(root.c_str());

	return nError == ERROR_CANCELLED && !EnvyFileSystem::EnumerationComplete(nError) && entries.size() < 3;
}

// A missing folder reports an error that does not justify pruning entries.
static bool test_list_directory_missing()
{
	std::vector<EnvyFileSystem::DirectoryEntry> entries;
	const DWORD nError = EnvyFileSystem::ListDirectory(L"C:\\EnvyNoSuchFolder_8f2c\\*.*", entries, []()
	                                                   { return false; });
	return entries.empty() && nError != ERROR_SUCCESS && nError != ERROR_NO_MORE_FILES;
}

void register_library_scan_state_smoke_tests(TestSuite& suite)
{
	suite.add_test("library_scan_path_relation_nesting", test_path_relation_nesting);
	suite.add_test("library_scan_list_directory_complete", test_list_directory_complete);
	suite.add_test("library_scan_list_directory_cancelled", test_list_directory_cancelled);
	suite.add_test("library_scan_list_directory_missing", test_list_directory_missing);
	suite.add_test("library_scan_generation_lifecycle", test_scan_generation_lifecycle);
	suite.add_test("library_scan_generation_coalesces_pending", test_scan_generation_coalesces_pending_requests);
	suite.add_test("library_scan_generation_request_during_scan",
	               test_scan_generation_request_during_scan_needs_next_pass);
	suite.add_test("library_scan_generation_join_unstarted", test_scan_generation_join_unstarted_request);
	suite.add_test("library_scan_generation_join_running", test_scan_generation_join_running_scan_issues_next);
	suite.add_test("library_scan_generation_interrupted_scan", test_scan_generation_interrupted_scan_stays_pending);
	suite.add_test("library_scan_generation_wraparound", test_scan_generation_wraparound);
	suite.add_test("library_scan_tick_elapsed_wraparound", test_tick_elapsed_wraparound);
	suite.add_test("library_scan_generation_concurrent_requests", test_scan_generation_concurrent_requests);
	suite.add_test("library_worker_lazy_then_maintenance", test_worker_runs_lazy_then_maintenance);
	suite.add_test("library_worker_cancel_during_lazy", test_worker_cancel_during_lazy_loading);
	suite.add_test("library_worker_cancelled_before_start", test_worker_cancelled_before_start);
	suite.add_test("library_worker_survives_ignored_second_start", test_single_worker_survives_ignored_second_start);
}
