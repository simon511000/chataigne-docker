/*
  ==============================================================================

    SequenceLayerFactory.h
    Created: 20 Nov 2016 3:09:30pm
    Author:  Ben Kuper

  ==============================================================================
*/

#ifndef SEQUENCELAYERFACTORY_H_INCLUDED
#define SEQUENCELAYERFACTORY_H_INCLUDED


#include "SequenceLayer.h"

class SequenceLayerDefinition
{
public:
	String type;
	std::function<SequenceLayer*(Sequence *)> createFunc;

	SequenceLayerDefinition(const String &_type, std::function<SequenceLayer*(Sequence *)> createFunc) :
		type(_type),
		createFunc(createFunc)
	{}
};

class SequenceLayerFactory
{
public:
	juce_DeclareSingleton(SequenceLayerFactory, true);

	OwnedArray<SequenceLayerDefinition> layerDefs;
	PopupMenu menu;

	SequenceLayerFactory();
	~SequenceLayerFactory() {}

	void buildPopupMenu();

	static SequenceLayer * showCreateMenu(Sequence * sequence)
	{
		int result = getInstance()->menu.show();
		if (result == 0) return nullptr;
		else
		{
			SequenceLayerDefinition * d = getInstance()->layerDefs[result - 1];//result 0 is no result
			return d->createFunc(sequence);
		}
	}

	static SequenceLayer * createSequenceLayer(Sequence * sequence, const String &inputType)
	{
		for (auto &d : getInstance()->layerDefs) if (d->type == inputType) return d->createFunc(sequence);
		return nullptr;
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequenceLayerFactory)
};



#endif  // SEQUENCELAYERFACTORY_H_INCLUDED
