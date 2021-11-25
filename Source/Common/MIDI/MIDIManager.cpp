/*
  ==============================================================================

	MIDIManager.cpp
	Created: 20 Dec 2016 12:33:33pm
	Author:  Ben

  ==============================================================================
*/

#include "ChataigneEngine.h"

juce_ImplementSingleton(MIDIManager)

MIDIManager::MIDIManager()
{

	midiRouterDefaultType = dynamic_cast<ChataigneEngine *>(Engine::mainEngine)->defaultBehaviors.addEnumParameter("MIDI Router Ouput Type","Choose the default type when choosing a MIDI Module as Router output");
	midiRouterDefaultType->addOption("Control Change", MIDIManager::CONTROL_CHANGE)->addOption("Note On", MIDIManager::NOTE_ON)->addOption("Note Off", MIDIManager::NOTE_OFF);

	startTimer(500); //check devices each half seconds
	checkDevices();
}

MIDIManager::~MIDIManager()
{
}

void MIDIManager::checkDevices()
{

	//INPUTS
	Array<MidiDeviceInfo> inputInfos = MidiInput::getAvailableDevices();
	Array<MIDIInputDevice*> inputDevicesToRemove;
	for (auto& d : inputs)
	{
		bool contains = false;
		for (auto& i : inputInfos)
		{
			if (i.identifier == d->id)
			{
				contains = true;
				break;
			}
		}

		if (!contains) inputDevicesToRemove.add(d);
	}

	for (auto& d : inputDevicesToRemove)
	{
		removeInputDevice(d);
	}

	for (auto& i : inputInfos)
	{
		addInputDeviceIfNotThere(i);
	}


	//OUTPUTS
	Array<MidiDeviceInfo> outputInfos = MidiOutput::getAvailableDevices();
	Array<MIDIOutputDevice*> outputDevicesToRemove;

	for (auto& d : outputs)
	{
		bool contains = false;
		for (auto& i : outputInfos)
		{
			if (i.identifier == d->id)
			{
				contains = true;
				break;
			}
		}

		if (!contains) outputDevicesToRemove.add(d);
	}

	for (auto& d : outputDevicesToRemove)
	{
		removeOutputDevice(d);
	}


	for (auto& i : outputInfos)
	{
		addOutputDeviceIfNotThere(i);
	}
}

void MIDIManager::addInputDeviceIfNotThere(const MidiDeviceInfo& info)
{
	if (getInputDeviceWithID(info.identifier) != nullptr) return;
	MIDIInputDevice* d = new MIDIInputDevice(info);
	inputs.add(d);

	NLOG("MIDI", "Device In Added : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&Listener::midiDeviceInAdded, d);
}

void MIDIManager::addOutputDeviceIfNotThere(const MidiDeviceInfo& info)
{
	if (getOutputDeviceWithID(info.identifier) != nullptr) return;
	MIDIOutputDevice* d = new MIDIOutputDevice(info);
	outputs.add(d);

	NLOG("MIDI", "Device Out Added : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&Listener::midiDeviceOutAdded, d);
}

void MIDIManager::removeInputDevice(MIDIInputDevice* d)
{
	inputs.removeObject(d, false);

	NLOG("MIDI", "Device In Removed : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&Listener::midiDeviceInRemoved, d);
	delete d;
}

void MIDIManager::removeOutputDevice(MIDIOutputDevice* d)
{
	outputs.removeObject(d, false);

	NLOG("MIDI", "Device Out Removed : " << d->name << " (ID : " << d->id << ")");

	listeners.call(&Listener::midiDeviceOutRemoved, d);
	delete d;
}

MIDIInputDevice* MIDIManager::getInputDeviceWithID(const String& id)
{
	for (auto& d : inputs) if (d->id == id) return d;
	return nullptr;
}

MIDIOutputDevice* MIDIManager::getOutputDeviceWithID(const String& id)
{
	for (auto& d : outputs) if (d->id == id) return d;
	return nullptr;
}

MIDIInputDevice* MIDIManager::getInputDeviceWithName(const String& name)
{
	for (auto& d : inputs) if (d->name == name) return d;
	return nullptr;
}

MIDIOutputDevice* MIDIManager::getOutputDeviceWithName(const String& name)
{
	for (auto& d : outputs) if (d->name == name) return d;
	return nullptr;
}

void MIDIManager::timerCallback()
{
	checkDevices();
}