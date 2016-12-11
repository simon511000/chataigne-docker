/*
  ==============================================================================

    SequenceLayer.cpp
    Created: 28 Oct 2016 8:15:05pm
    Author:  bkupe

  ==============================================================================
*/

#include "SequenceLayer.h"
#include "SequenceLayerPanel.h"
#include "SequenceLayerTimeline.h"

SequenceLayer::SequenceLayer(Sequence * _sequence, const String &name) :
	BaseItem(name),
	sequence(_sequence)
{
	jassert(sequence != nullptr);

	uiHeight = addIntParameter("UI Height", "Height of the layer in the editor", 60, 20, 300);
	sequence = _sequence;
	sequence->addSequenceListener(this);

}

SequenceLayer::~SequenceLayer()
{
	sequence->removeSequenceListener(this);
}

var SequenceLayer::getJSONData()
{
	var data = BaseItem::getJSONData();
	data.getDynamicObject()->setProperty("type", getTypeString());
	return data;
}

SequenceLayerPanel * SequenceLayer::getPanel()
{
	return new SequenceLayerPanel(this);
}

SequenceLayerTimeline * SequenceLayer::getTimelineUI()
{
	return new SequenceLayerTimeline(this);
}

