/*
 *    Copyright 2014 Kai Pastor
 *
 *    This file is part of OpenOrienteering.
 *
 *    OpenOrienteering is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    OpenOrienteering is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with OpenOrienteering.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "autosave.h"
#include "autosave_p.h"

#include "../settings.h"

AutosavePrivate::AutosavePrivate(Autosave& document)
: document(document)
, autosave_needed(false)
{
	autosave_timer = new QTimer(this);
	autosave_timer->setSingleShot(true);
	connect(autosave_timer, SIGNAL(timeout()), this, SLOT(autosave()));
	connect(&Settings::getInstance(), SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));
	settingsChanged();
}

AutosavePrivate::~AutosavePrivate()
{
	// nothing, not inlined
}

void AutosavePrivate::settingsChanged()
{
	// Normally, the autosave interval can be stored as an integer.
	// It is loaded as a double here to allow for faster unit testing.
	autosave_interval = Settings::getInstance().getSetting(Settings::General_AutosaveInterval).toDouble() * 60000;
	if (autosave_interval < 1000)
	{
		// stop autosave
		autosave_interval = 0;
		autosave_timer->stop();
	}
	else if (autosave_needed && !autosave_timer->isActive())
	{
		// start autosave
		autosave_timer->setInterval(autosave_interval);
		autosave_timer->start();
	}
}

bool AutosavePrivate::autosaveNeeded()
{
	return autosave_needed;
}

void AutosavePrivate::setAutosaveNeeded(bool needed)
{
	autosave_needed = needed;
	if (autosave_interval)
	{
		// autosaving enabled
		if (autosave_needed && !autosave_timer->isActive())
		{
			autosave_timer->setInterval(autosave_interval);
			autosave_timer->start();
		}
		else if (!autosave_needed && autosave_timer->isActive())
		{
			autosave_timer->stop();
		}
	}
}

void AutosavePrivate::autosave()
{
	Autosave::AutosaveResult result = document.autosave();
	if (autosave_interval)
	{
		switch (result)
		{
		case Autosave::TemporaryFailure:
			autosave_timer->setInterval(5000);
			autosave_timer->start();
			break;
		case Autosave::Success:
		case Autosave::PermanentFailure:
			autosave_timer->setInterval(autosave_interval);
			autosave_timer->start();
			break;
		default:
			Q_ASSERT(false && "Return value of Autosave() is not a valid Autosave::AutosaveResult.");
		}
	}
}



Autosave::Autosave()
: autosave_controller(new AutosavePrivate(*this))
{
	// Nothing, not inlined
}

Autosave::~Autosave()
{
	// Nothing, not inlined
}

void Autosave::setAutosaveNeeded(bool needed)
{
	autosave_controller->setAutosaveNeeded(needed);
}