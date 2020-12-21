/*
  ==============================================================================

    StateManager.cpp
    Created: 28 Oct 2016 8:19:15pm
    Author:  bkupe

  ==============================================================================
*/

#include "StateManager.h"
#include "Common/Processor/Action/Action.h"
#include "Common/Processor/Mapping/Mapping.h"
#include "Common/Processor/Action/Condition/conditions/StandardCondition/StandardCondition.h"

juce_ImplementSingleton(StateManager)

StateManager::StateManager() :
BaseManager<State>("States"),
module(this),
stm(this)
{
	itemDataType = "State";
	helpID = "StateMachine";

	addChildControllableContainer(&stm);
	stm.hideInEditor = true;
	stm.addBaseManagerListener(this);

	addChildControllableContainer(&commentManager);
	Engine::mainEngine->addEngineListener(this);
}

StateManager::~StateManager()
{
	if(Engine::mainEngine != nullptr) Engine::mainEngine->removeEngineListener(this);
}


void StateManager::clear()
{
	stm.clear();
	commentManager.clear();
	BaseManager::clear();
}

void StateManager::setStateActive(State * s)
{
	Array<State *> linkedStates = getLinkedStates(s);
	for (auto &ss : linkedStates)
	{
		ss->active->setValue(false);
	}
}

void StateManager::addItemInternal(State * s, var data)
{
	s->addStateListener(this);
	if (!Engine::mainEngine->isLoadingFile)
	{
		s->active->setValue(true);
	}
}

void StateManager::removeItemInternal(State * s)
{
	s->removeStateListener(this);

	Array<State*> avoid;
	avoid.add(s);
	Array<State*> linkedStates;
	for (auto& ls : linkedStates) checkStartActivationOverlap(ls, avoid);
}

Array<UndoableAction *> StateManager::getRemoveItemUndoableAction(State * item)
{
	Array<UndoableAction *> result;
	result.addArray(stm.getRemoveAllLinkedTransitionsAction(item));
	result.addArray(BaseManager::getRemoveItemUndoableAction(item));

	return result;
}

Array<UndoableAction*> StateManager::getRemoveItemsUndoableAction(Array<State*> itemsToRemove)
{
	Array<UndoableAction *> result;
	for (auto &i : itemsToRemove) result.addArray(stm.getRemoveAllLinkedTransitionsAction(i));
	result.addArray(BaseManager::getRemoveItemsUndoableAction(itemsToRemove));
	return result;
}

void StateManager::stateActivationChanged(State * s)
{
	if (s->active->boolValue())
	{
		setStateActive(s);
	}
}

void StateManager::stateStartActivationChanged(State* s)
{
	checkStartActivationOverlap(s);
}

void StateManager::checkStartActivationOverlap(State* s, Array<State*> statesToAvoid)
{
	if (s == nullptr) return;

	Array<State*> linkedStates = getLinkedStates(s, &statesToAvoid);
	linkedStates.add(s);
	Array<State*> forceActiveStates;
	for (auto& ss : linkedStates)
	{
		if (ss->loadActivationBehavior->getValueDataAsEnum<State::LoadBehavior>() == State::ACTIVE) forceActiveStates.add(ss);
		else ss->clearWarning();
	}

	if (forceActiveStates.size() > 1)
	{
		LOGWARNING("Multiple linked states are set to activate on load, it may lead to unexpected behaviors");
		for (auto& fs : forceActiveStates) fs->setWarningMessage("Concurrent Active on Load state");
	}
	else if (forceActiveStates.size() > 0) forceActiveStates[0]->clearWarning();
}

void StateManager::itemAdded(StateTransition * s)
{
	if (!Engine::mainEngine->isLoadingFile)
	{
		if(s->sourceState->active->boolValue()) setStateActive(s->sourceState);
		else if(s->destState->active->boolValue()) setStateActive(s->destState);

		checkStartActivationOverlap(s->sourceState);
	}
}

void StateManager::itemsAdded(Array<StateTransition*> states)
{
	if (!Engine::mainEngine->isLoadingFile)
	{
		for (auto& s : states)
		{
			if (s->sourceState->active->boolValue()) setStateActive(s->sourceState);
			else if (s->destState->active->boolValue()) setStateActive(s->destState);

			checkStartActivationOverlap(s->sourceState);
		}
	}
}

void StateManager::itemRemoved(StateTransition* s)
{
	if (!Engine::mainEngine->isClearing)
	{
		Array<State*> avoidStates;
		avoidStates.add(s->sourceState, s->destState);
		checkStartActivationOverlap(s->sourceState, avoidStates);
		checkStartActivationOverlap(s->destState, avoidStates);
	}
}

void StateManager::itemsRemoved(Array<StateTransition*> states)
{
	if (!Engine::mainEngine->isClearing)
	{
		for (auto& s : states)
		{
			Array<State*> avoidStates;
			avoidStates.add(s->sourceState, s->destState);
			checkStartActivationOverlap(s->sourceState, avoidStates);
			checkStartActivationOverlap(s->destState, avoidStates);
		}
	}
}


State * StateManager::showMenuAndGetState()
{
	PopupMenu menu;
	StateManager * sm = StateManager::getInstance();
	for (int i = 0; i < sm->items.size(); ++i)
	{
		menu.addItem(1+i,sm->items[i]->niceName);
	}

	int result = menu.show();
	if (result <= 0) return nullptr;
	return sm->items[result - 1];
}

Action * StateManager::showMenuAndGetAction()
{
	PopupMenu menu;
	StateManager * sm = StateManager::getInstance();

	Array<Action*> actions;

	for (auto & s : sm->items)
	{
		PopupMenu sMenu;
		for (auto & p : s->pm.items)
		{
			if (p->type == Processor::ACTION)
			{
				actions.add((Action*)p);
				sMenu.addItem(actions.size(), p->niceName);
			}
		}
		menu.addSubMenu(s->niceName, sMenu);
	}

	int result = menu.show();
	if (result <= 0) return nullptr;
	return actions[result-1];
}

Mapping * StateManager::showMenuAndGetMapping()
{
	PopupMenu menu;
	StateManager* sm = StateManager::getInstance();

	Array<Mapping*> mappings;

	for (auto& s : sm->items)
	{
		PopupMenu sMenu;
		for (auto& p : s->pm.items)
		{
			if (p->type == Processor::MAPPING)
			{
				mappings.add((Mapping*)p);
				sMenu.addItem(mappings.size(), p->niceName);
			}
		}
		menu.addSubMenu(s->niceName, sMenu);
	}

	int result = menu.show();
	if (result <= 0) return nullptr;
	return mappings[result - 1];
}

StandardCondition* StateManager::showMenuAndGetToggleCondition()
{
	PopupMenu menu;
	StateManager* sm = StateManager::getInstance();

	Array<StandardCondition*> conditions;

	for (auto& s : sm->items)
	{
		PopupMenu sMenu;
		for (auto& p : s->pm.items)
		{
			if (p->type == Processor::ACTION)
			{
				Action* a = (Action*)p;
				PopupMenu aMenu;
				for (auto& c : a->cdm.items)
				{
					if (StandardCondition* sc = dynamic_cast<StandardCondition*>(c))
					{
						if (sc->comparator != nullptr && sc->toggleMode->boolValue())
						{
							conditions.add(sc);
							aMenu.addItem(conditions.size(), sc->niceName);
						}
					}
				}
				sMenu.addSubMenu(a->niceName,  aMenu);
			}
		}
		menu.addSubMenu(s->niceName, sMenu);
	}

	int result = menu.show();
	if (result <= 0) return nullptr;
	return conditions[result - 1];
}


Array<State *> StateManager::getLinkedStates(State * s, Array<State *> * statesToAvoid)
{
	Array<State *> sAvoid;
	if (statesToAvoid == nullptr)
	{
		statesToAvoid = &sAvoid;
	}

	statesToAvoid->add(s);

	Array<State *> result;

	Array<State *> linkStates = stm.getAllStatesLinkedTo(s);
	for (auto &ss : linkStates)
	{
		if (statesToAvoid->contains(ss)) continue;
		result.add(ss);
		statesToAvoid->add(ss);
		Array<State *> linkedSS = getLinkedStates(ss, statesToAvoid);
		result.addArray(linkedSS);
	}

	return result;
}


var StateManager::getJSONData()
{
	var data = BaseManager::getJSONData();

	var tData = stm.getJSONData();
	if (!tData.isVoid() && tData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty(stm.shortName,tData);
	var cData = commentManager.getJSONData();
	if (!cData.isVoid() && cData.getDynamicObject()->getProperties().size() > 0) data.getDynamicObject()->setProperty(commentManager.shortName, cData);
	return data;
}

void StateManager::loadJSONDataInternal(var data)
{
	BaseManager::loadJSONDataInternal(data);
	stm.loadJSONData(data.getProperty(stm.shortName, var()));
	commentManager.loadJSONData(data.getProperty(commentManager.shortName, var()));

	for (auto& s : items)
	{
		State::LoadBehavior b = s->loadActivationBehavior->getValueDataAsEnum<State::LoadBehavior>();
		if (b == State::ACTIVE)
		{
			s->active->setValue(true);
			setStateActive(s);
			checkStartActivationOverlap(s);
		}
		
	}
}

void StateManager::endLoadFile()
{
	for (auto& s : items) if (s->active->boolValue()) s->active->setValue(true, false, true);
}
