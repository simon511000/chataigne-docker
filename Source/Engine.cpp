/*
 ==============================================================================

 Engine.cpp
 Created: 2 Apr 2016 11:03:21am
 Author:  Martin Hermant

 ==============================================================================
 */

#include "Engine.h"
#include "InputManager.h"
#include "OutputManager.h"
#include "StateManager.h"
#include "SequenceManager.h"
#include "FlapLogger.h"
#include "PresetManager.h"
#include "StringUtil.h"


const char* const filenameSuffix = ".flap";
const char* const filenameWildcard = "*.flap";

Engine::Engine():FileBasedDocument (filenameSuffix,
                                    filenameWildcard,
                                    "Load a Flap Document",
                                    "Save a Flap Document"){
  
  //to move into engine
  Logger::setCurrentLogger(FlapLogger::getInstance());

}


Engine::~Engine(){

//delete managers
  
InputManager::deleteInstance();

  PresetManager::deleteInstance();
  FlapLogger::deleteInstance();
  Logger::setCurrentLogger(nullptr);

}

void Engine::parseCommandline(const String & commandLine){

  for (auto & c:StringUtil::parseCommandLine(commandLine)){

    if(c.command== "f"|| c.command==""){
      if(c.args.size()==0){
        LOG("no file provided for command : "+c.command);
        jassertfalse;
        continue;
      }
      String fileArg = c.args[0];
      if (File::isAbsolutePath(fileArg)) {
        File f(fileArg);
        if (f.existsAsFile()) loadDocument(f);
      }
      else {
        NLOG("Engine","File : " << fileArg << " not found.");
      }
    }

  }

}

void Engine::clear(){
  

  PresetManager::getInstance()->clear();
 
  //clear managers

  changed();    //fileDocument
}
