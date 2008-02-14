// qtractorVstPlugin.h
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

#ifndef __qtractorVstPlugin_h
#define __qtractorVstPlugin_h

#include "qtractorPlugin.h"

// Allow VST 2.3 compability mode.
#define VST_2_4_EXTENSIONS 0

#if !defined(__WIN32__) && !defined(_WIN32) && !defined(WIN32)
#define __cdecl
#endif

#include <aeffectx.h>


//----------------------------------------------------------------------------
// qtractorVstPluginType -- LADSPA plugin type instance.
//

class qtractorVstPluginType : public qtractorPluginType
{
public:

	// Forward declarations.
	class Effect;

	// Constructor.
	qtractorVstPluginType(qtractorPluginFile *pFile, Effect *pEffect = NULL)
		: qtractorPluginType(pFile, 0, qtractorPluginType::Vst),
			m_pEffect(pEffect), m_iFlagsEx(0) {}

	// Destructor.
	~qtractorVstPluginType()
		{ close(); }

	// Derived methods.
	bool open();
	void close();

	// Specific accessors.
	Effect *effect() const { return m_pEffect; }

	// Factory method (static)
	static qtractorVstPluginType *createType(qtractorPluginFile *pFile);

	// Effect instance method (static)
	static AEffect *vst_effect(qtractorPluginFile *pFile);

	// VST host dispatcher.
	int vst_dispatch(
		long opcode, long index, long value, void *ptr, float opt) const;

protected:

	// VST flag inquirer.
	bool vst_canDo(const char *pszCanDo) const;

private:

	// VST descriptor reference.
	Effect      *m_pEffect;
	unsigned int m_iFlagsEx;
};


//----------------------------------------------------------------------------
// qtractorVstPlugin -- VST plugin instance.
//

class qtractorVstPlugin : public qtractorPlugin
{
public:

	// Constructor.
	qtractorVstPlugin(qtractorPluginList *pList,
		qtractorVstPluginType *pVstType);

	// Destructor.
	~qtractorVstPlugin();

	// Channel/intsance number accessors.
	void setChannels(unsigned short iChannels);

	// Do the actual (de)activation.
	void activate();
	void deactivate();

	// GUI Editor stuff.
	void openEditor(QWidget *pParent);
	void closeEditor();
	void idleEditor();

	// GUI editor visibility state.
	void setEditorVisible(bool bVisible);
	bool isEditorVisible() const;

	void setEditorTitle(const QString& sTitle);

	// The main plugin processing procedure.
	void process(float **ppIBuffer, float **ppOBuffer, unsigned int nframes);

	// Specific accessors.
	AEffect *vst_effect(unsigned short iInstance) const;

	// VST host dispatcher.
	int vst_dispatch(unsigned short iInstance,
		long opcode, long index, long value, void *ptr, float opt) const;

	// Idle timer flag accessor.
	void setIdleTimer(bool bIdleTimer)
		{ m_bIdleTimer = bIdleTimer; }
	bool isIdleTimer() const
		{ return m_bIdleTimer; }

	// Our own editor widget accessor.
	QWidget *editorWidget() const;

	// Global VST plugin lookup.
	static qtractorVstPlugin *findPlugin(AEffect *pVstEffect);

	// Idle editor (static).
	static void idleEditorAll();

	// Idle timer (static).
	static void idleTimerAll();

protected:

	// Editor widget forward decls.
	class EditorWidget;

private:

	// Instance variables.
	qtractorVstPluginType::Effect **m_ppEffects;

	// Audio I/O buffer pointers.
	float **m_ppIBuffer;
	float **m_ppOBuffer;

	// Idle timer flag.
	bool m_bIdleTimer;

	// Our own editor widget (parent frame).
	EditorWidget *m_pEditorWidget;

	// Singleton list of VST plugins.
	static QList<qtractorVstPlugin *> g_vstPlugins;
};


//----------------------------------------------------------------------------
// qtractorVstPluginParam -- VST plugin control input port instance.
//

class qtractorVstPluginParam : public qtractorPluginParam
{
public:

	// Constructors.
	qtractorVstPluginParam(qtractorVstPlugin *pVstPlugin,
		unsigned long iIndex);

	// Destructor.
	~qtractorVstPluginParam();

	// Port range hints predicate methods.
	bool isBoundedBelow() const;
	bool isBoundedAbove() const;
	bool isDefaultValue() const;
	bool isLogarithmic() const;
	bool isSampleRate() const;
	bool isInteger() const;
	bool isToggled() const;
	bool isDisplay() const;

	// Current display value.
	QString display() const;

	// Current parameter value.
	void setValue(float fValue);
	float value() const;

private:

	VstParameterProperties m_props;
};


#endif  // __qtractorVstPlugin_h

// end of qtractorVstPlugin.h