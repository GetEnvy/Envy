//
// Scheduler.h
//
// This file is part of Envy (getenvy.com) © 2016
// Portions copyright PeerProject 2008-2014 and Shareaza 2002-2007
//
// Envy is free software. You may redistribute and/or modify it
// under the terms of the GNU Affero General Public License
// as published by the Free Software Foundation (fsf.org);
// version 3 or later at your option. (AGPLv3)
//
// Envy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Affero General Public License 3.0 for details:
// (http://www.gnu.org/licenses/agpl.html)
//

#pragma once

class CXMLElement;

// Add new tasks here: (In order)
enum ScheduleTask
{
	BANDWIDTH_FULL		= 0x1,
	BANDWIDTH_LIMITED	= 0x2,
	BANDWIDTH_STOP		= 0x4,
	SYSTEM_EXIT 		= 0x8,
	SYSTEM_SHUTDOWN 	= 0x10,
	SYSTEM_DISCONNECT	= 0x20,
	SYSTEM_NOTICE		= 0x40
};

enum DayOfWeek
{
	MONDAY		= 0x1,
	TUESDAY		= 0x2,
	WEDNESDAY 	= 0x4,
	THURSDAY	= 0x8,
	FRIDAY		= 0x10,
	SATURDAY	= 0x20,
	SUNDAY		= 0x40
};


//////////////////////////////////////////////////////////////////////
// CScheduleTask class: Represents a scheduled task
//

class CScheduleTask
{
public:
	CScheduleTask(BOOL bCreate = TRUE);
	CScheduleTask(const CScheduleTask& pItem);
	virtual ~CScheduleTask();

public:
	unsigned int	m_nDays;			// Will have a combination of DayOfWeek
	unsigned int	m_nAction;			// Will have one of ScheduleTask values plus 0 as invalid state indicator
	bool			m_bSpecificDays;	// Task is scheduled for everyday or just today
	CString			m_sDescription;		// Optional task description
	CTime			m_tScheduleDateTime;// Time the task is scheduled for
	bool			m_bActive;			// Task should be executed or not
	bool			m_bExecuted;		// Task is executed or not
	bool			m_bLimitedNetworks;	// Network is limited to G2 or not (in SCHEDULE_LIMITED_SPEED)
	int				m_nLimitDown;		// Downstream bandwidth limit
	int				m_nLimitUp;			// Upstream bandwidth limit
	GUID			m_pGUID;			// GUID for each scheduled item

public:
	CXMLElement*	ToXML();
	BOOL			FromXML(CXMLElement* pXML);
	void			Serialize(CArchive& ar, int nVersion);
};


//////////////////////////////////////////////////////////////////////
// CScheduler class: Controls scheduler operations
//

class CScheduler
{
public:
	CScheduler();
	virtual ~CScheduler();

public:
	static LPCTSTR					xmlns;

	// Lock used when objects read/write to/from m_pScheduleItems
	mutable CCriticalSection		m_pSection;

protected:
	CList< CScheduleTask* >			m_pScheduleTasks;

public:

	// To iterate through m_pScheduleItems
	inline POSITION	GetIterator() const
	{
		return m_pScheduleTasks.GetHeadPosition();
	}

	inline CScheduleTask*	GetNext(POSITION& pos) const
	{
		return m_pScheduleTasks.GetNext( pos );
	}

	inline int		GetCount() const
	{
		return m_pScheduleTasks.GetCount();
	}

	// Checks to see pItem exists in m_pScheduleItems or not, by comparing GUID values
	bool			Check(CScheduleTask* pSchTask) const
	{
		return pSchTask != NULL && GetGUID( pSchTask->m_pGUID ) != NULL;
	}

	// It is called regularly by timers to see if any scheduled item should be executed
	// Also sets Settings.Scheduler.Enable to indiate globally if any item is going to be executed
	void			CheckSchedule();		// Called regularly by timers to check if any scheduled item should be executed, also sets Settings.Scheduler.Enable
	void			HangUpConnection(); 	// Used to disconnect dial up connection
	bool			ShutDownComputer(); 	// Used to shut down computer
	bool			SetShutdownRights();	// Called by Load() to get shutdown privilege for the process
	bool			IsScheduledTimePassed(CScheduleTask* pSchTask) const;	// Called by CheckSchedule() to see if Now is grater than Then or not.
	int				ScheduleFromToday(CScheduleTask* pSchTask) const;		// Checks to see if task should be executed today 0, in the past -1, or later 1.

	void			Add(CScheduleTask* pSchTask);		// Adds a new task to m_pScheduleItems after giving it a GUID
	void			Remove(CScheduleTask* pSchTask);	// Removes a task from m_pScheduleItems if it exists
	void			Clear();							// Clears all m_pScheduleItems items
	BOOL			Save();
	BOOL			Load();
	BOOL			Import(LPCTSTR pszFile);

	// Calculates hours remaining to execution of a combination of scheduled tasks
	// Example: nEventCombination = BANDWIDTH_FULL | SYSTEM_DISCONNECT
	LONGLONG		GetHoursTo(unsigned int nTaskCombination);

protected:
	CXMLElement*	ToXML(BOOL bTasks);
	BOOL			FromXML(CXMLElement* pXML);
	void			Serialize(CArchive& ar);

	void			ExecuteScheduledTask(CScheduleTask* pItem);	// Called by CheckSchedule() to execute a task

	CScheduleTask*	GetGUID(const GUID& pGUID) const;
};

extern CScheduler Scheduler;
