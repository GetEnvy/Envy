// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <windows.h>

namespace EnvyLibraryScan
{
enum class Phase
{
	Pending,  // Requested, no scan has picked the request up yet
	Scanning, // A scan covering the request is running
	Completed // A scan covering the request has finished
};

// Generations are monotonic LONG counters that may wrap; compare them by
// signed distance so a wrapped counter still orders correctly.
inline bool Reached(LONG nValue, LONG nTarget)
{
	return static_cast<LONG>(static_cast<ULONG>(nValue) - static_cast<ULONG>(nTarget)) >= 0;
}

// Tracks explicit full-scan requests so a waiter can tell "queued" from
// "running" from "done" for the scan it asked for. Requests issued before a
// scan starts are coalesced into that scan; a request issued while a scan is
// running needs the next one, because the running scan may already have passed
// the folders that changed. Only one worker calls Begin()/Complete().
class Generation
{
public:
	Generation() noexcept
	    : m_nRequested(0)
	    , m_nStarted(0)
	    , m_nCompleted(0)
	{
	}

	// Any thread. Returns the generation whose completion satisfies the request.
	LONG Request() noexcept
	{
		return InterlockedIncrement(&m_nRequested);
	}

	// Any thread. Returns the generation a new waiter should track: joins an
	// existing request that has not started yet, because its scan will cover
	// changes already made, and stacks a new request only when nothing is
	// pending or the pending scan is already running. Callers that change
	// shared folders and then ask for a scan (Library.Update( true ) followed
	// by the dialog's RequestScan()) therefore trigger one full pass instead
	// of two.
	LONG RequestOrJoin() noexcept
	{
		for (;;)
		{
			const LONG nRequested = Load(m_nRequested);
			const LONG nCompleted = Load(m_nCompleted);
			const LONG nStarted = Load(m_nStarted);
			if (nRequested != nCompleted && nRequested != nStarted)
				return nRequested; // Unstarted request: its scan covers our changes
			const LONG nNext = static_cast<LONG>(static_cast<ULONG>(nRequested) + 1u);
			if (InterlockedCompareExchange(&m_nRequested, nNext, nRequested) == nRequested)
				return nNext;
			// A request landed concurrently: retry with the new value.
		}
	}

	// Worker. Returns true and the generation to complete when a request is pending.
	bool Begin(LONG& nGeneration) noexcept
	{
		nGeneration = Load(m_nRequested);
		if (nGeneration == Load(m_nCompleted))
			return false;
		InterlockedExchange(&m_nStarted, nGeneration);
		return true;
	}

	// Worker. Only called after a scan that ran to the end; an interrupted scan
	// leaves the request pending.
	void Complete(LONG nGeneration) noexcept
	{
		InterlockedExchange(&m_nCompleted, nGeneration);
	}

	Phase GetPhase(LONG nTarget) const noexcept
	{
		if (Reached(Load(m_nCompleted), nTarget))
			return Phase::Completed;
		if (Reached(Load(m_nStarted), nTarget))
			return Phase::Scanning;
		return Phase::Pending;
	}

private:
	static LONG Load(const volatile LONG& nValue) noexcept
	{
		return InterlockedCompareExchange(const_cast<volatile LONG*>(&nValue), 0, 0);
	}

	volatile LONG m_nRequested;
	volatile LONG m_nStarted;
	volatile LONG m_nCompleted;
};

// Wrap-safe "at least nInterval ms elapsed since nSince" for GetTickCount() values.
inline bool Elapsed(DWORD nNow, DWORD nSince, DWORD nInterval)
{
	return nNow - nSince >= nInterval;
}

// Body of the single library worker thread: finish deferred loading, then
// run maintenance passes until cancellation. Doing both on one thread means
// the end of deferred loading can never end the only thread that scans shared
// folders.
template<class Worker>
void RunWorker(Worker& worker)
{
	if (!worker.IsThreadEnabled())
		return;

	worker.CompleteLazyLoading();

	while (worker.IsThreadEnabled())
		worker.RunMaintenancePass();
}
} // namespace EnvyLibraryScan
