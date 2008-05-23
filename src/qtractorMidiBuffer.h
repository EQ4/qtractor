// qtractorMidiBuffer.h
//
/****************************************************************************
   Copyright (C) 2005-2008, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __qtractorMidiBuffer_h
#define __qtractorMidiBuffer_h

#include "qtractorList.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <alsa/asoundlib.h>


// Forward declarations.
class qtractorSession;
class qtractorPluginList;


//----------------------------------------------------------------------
// class qtractorMidiBuffer -- MIDI event FIFO buffer/cache declaration.
//

class qtractorMidiBuffer
{
public:

	// Minimem buffer size
	enum { MinBufferSize = 0x400 };

	// Constructor.
	qtractorMidiBuffer(unsigned int iBufferSize = MinBufferSize) :
		m_pBuffer(NULL), m_iBufferSize(0), m_iBufferMask(0),
		m_iWriteIndex(0), m_iReadIndex(0)
	{
		// Adjust size of nearest power-of-two, if necessary.
		m_iBufferSize = MinBufferSize;
		while (m_iBufferSize < iBufferSize)
			m_iBufferSize <<= 1;
		m_iBufferMask = (m_iBufferSize - 1);
		m_pBuffer = new snd_seq_event_t [m_iBufferSize];
	}
		
	// Destructor.
	~qtractorMidiBuffer() { if (m_pBuffer) delete [] m_pBuffer; }

	// Implementation properties.
	unsigned int bufferSize() const { return m_iBufferSize; }

	// Clears the buffer.
	void clear() { m_iReadIndex = m_iWriteIndex = 0; }	

	// Returns nonzero if there aren't any events available.
	bool isEmpty() const { return (m_iReadIndex == m_iWriteIndex); }

	// Returns a pointer to the first of the output events. 
	snd_seq_event_t *peek() const
		{ return (isEmpty() ? NULL : &m_pBuffer[m_iReadIndex]); }

	// Read next event from buffer.
	snd_seq_event_t *next()
	{
		if (!isEmpty())	++m_iReadIndex &= m_iBufferMask;
		return peek();
	}

	// Read event from buffer.
	snd_seq_event_t *pop()
	{
		unsigned int iReadIndex = m_iReadIndex;
		if (iReadIndex == m_iWriteIndex)
			return NULL;
		m_iReadIndex = (iReadIndex + 1) & m_iBufferMask;
		return &m_pBuffer[iReadIndex];
	}

	// Write event to buffer.
	bool push(snd_seq_event_t *pEvent, unsigned long iTick = 0)
	{
		unsigned int iWriteIndex = (m_iWriteIndex + 1) & m_iBufferMask;
		if (iWriteIndex == m_iReadIndex)
			return false;
		m_pBuffer[m_iWriteIndex] = *pEvent;
		m_pBuffer[m_iWriteIndex].time.tick = iTick;
		m_iWriteIndex = iWriteIndex;
		return true;
	}

	// Write event to buffer (ordered).
	bool insert(snd_seq_event_t *pEvent, unsigned long iTick = 0)
	{
		unsigned int iWriteIndex = (m_iWriteIndex + 1) & m_iBufferMask;
		if (iWriteIndex == m_iReadIndex)
			return false;
		unsigned int i = m_iWriteIndex;
		unsigned int j = i;
		for (;;) {
			--i &= m_iBufferMask;
			if (j == m_iReadIndex
				|| iTick >= m_pBuffer[i].time.tick) {
				m_pBuffer[j] = *pEvent;
				m_pBuffer[j].time.tick = iTick;
				break;
			}
			m_pBuffer[j] = m_pBuffer[i];
			j = i;
		}
		m_iWriteIndex = iWriteIndex;
		return true;
	}

	// Returns number of events currently available.
	unsigned int count() const
	{
		unsigned int iWriteIndex = m_iWriteIndex;
		unsigned int iReadIndex  = m_iReadIndex;
		if (iWriteIndex > iReadIndex) {
			return (iWriteIndex - iReadIndex);
		} else {
			return (iWriteIndex - iReadIndex + m_iBufferSize) & m_iBufferMask;
		}
	}

private:

	// Instance variables.
	snd_seq_event_t *m_pBuffer;
	unsigned int m_iBufferSize;
	unsigned int m_iBufferMask;
	unsigned int m_iWriteIndex;
	unsigned int m_iReadIndex;
};


//----------------------------------------------------------------------
// class qtractorMidiManager -- MIDI internal plugin list manager.
//

class qtractorMidiManager : public qtractorList<qtractorMidiManager>::Link
{
public:

	// Constructor.
	qtractorMidiManager(qtractorSession *pSession, qtractorPluginList *pPluginList,
		unsigned int iBufferSize = qtractorMidiBuffer::MinBufferSize);

	// Destructor.
	~qtractorMidiManager();

	// Implementation properties.
	qtractorSession *session() const
		{ return m_pSession; }
	qtractorPluginList *pluginList() const
		{ return m_pPluginList; }

	unsigned int bufferSize() const
		{ return m_queuedBuffer.bufferSize(); }

	// Clears the buffer.
	void clear() { m_iBuffer = 0; }

	// Event buffer accessor. 
	snd_seq_event_t *buffer() const	{ return m_pBuffer; }

	// Returns number of events result of process.
	unsigned int count() const { return m_iBuffer; }

	// Direct buffering.
	bool direct(snd_seq_event_t *pEvent);

	// Queued buffering.
	bool queued(snd_seq_event_t *pEvent);

	// Process buffers.
	void process(unsigned long iTimeStart, unsigned long iTimeEnd);

	// Resets all buffering.
	void reset();

	// Factory (proxy) methods.
	static qtractorMidiManager *createMidiManager(
		qtractorPluginList *pPluginList);
	static void deleteMidiManager(
		qtractorMidiManager *pMidiManager);

protected:

	// Instance variables
	qtractorSession    *m_pSession;
	qtractorPluginList *m_pPluginList;

	qtractorMidiBuffer  m_directBuffer;
	qtractorMidiBuffer  m_queuedBuffer;
	qtractorMidiBuffer  m_postedBuffer;

	snd_seq_event_t    *m_pBuffer;
	unsigned int        m_iBuffer;

	int m_iPendingBankMSB;
	int m_iPendingBankLSB;
	int m_iPendingProg;
};


#endif  // __qtractorMidiBuffer_h

// end of qtractorMidiBuffer.h