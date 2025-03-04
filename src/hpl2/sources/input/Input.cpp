/*
 * Copyright © 2011-2020 Frictional Games
 * 
 * This file is part of Amnesia: A Machine For Pigs.
 * 
 * Amnesia: A Machine For Pigs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 

 * Amnesia: A Machine For Pigs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: A Machine For Pigs.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "input/Input.h"
#include "system/LowLevelSystem.h"
#include "input/Mouse.h"
#include "input/Keyboard.h"
#include "input/Gamepad.h"
#include "input/LowLevelInput.h"
#include "input/Action.h"
#include "input/ActionKeyboard.h"
#include "input/ActionMouseButton.h"

namespace hpl 
{
	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	cInput::cInput(iLowLevelInput *apLowLevelInput) : iUpdateable("HPL_Input")
	{
		mpLowLevelInput = apLowLevelInput;

		mpKeyboard = mpLowLevelInput->CreateKeyboard();
		mpMouse = mpLowLevelInput->CreateMouse();

		mlstInputDevices.push_back(mpMouse);
		mlstInputDevices.push_back(mpKeyboard);

		RefreshGamepads();
	}

	//-----------------------------------------------------------------------
	
	cInput::~cInput()
	{
		Log("Exiting Input Module\n");
		Log("--------------------------------------------------------\n");

		STLMapDeleteAll(m_mapActions);
		STLDeleteAll(mlstGamepads);

		if(mpKeyboard)hplDelete(mpKeyboard);
		if(mpMouse)hplDelete(mpMouse);

		Log("--------------------------------------------------------\n\n");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS				
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cInput::Update(float afTimeStep)
	{
		
		mpLowLevelInput->BeginInputUpdate();
		
		for(tInputDeviceListIt it = mlstInputDevices.begin(); it!= mlstInputDevices.end();++it)
		{
			(*it)->Update();
		}

		mpLowLevelInput->EndInputUpdate();

		for(tActionMapIt it = m_mapActions.begin(); it!= m_mapActions.end();++it)
		{
			it->second->Update(afTimeStep);
		}
	}
	
	//-----------------------------------------------------------------------
	
	cAction * cInput::CreateAction(const tString& asName, int alId)
	{
		cAction *pAction = hplNew( cAction, (asName, alId,this) );

		tActionMap::value_type val = tActionMap::value_type(asName,pAction);
		m_mapActions.insert(val);

		tActionIdMap::value_type val2 = tActionIdMap::value_type(alId,pAction);
		m_mapActionIds.insert(val2);
		
		return pAction;
	}
	
	//-----------------------------------------------------------------------
	
	bool cInput::IsTriggerd(const tString& asName)
	{
		cAction *pAction = GetAction(asName);
		if(pAction==NULL){return false;}//Log("doesn't exist!!!");return false;}

		return pAction->IsTriggerd();
	}

	bool cInput::IsTriggerd(int alId)
	{
		cAction *pAction = GetAction(alId);
		if(pAction==NULL){return false;}

		return pAction->IsTriggerd();
	}

	//-----------------------------------------------------------------------

	bool cInput::WasTriggerd(const tString& asName)
	{
		cAction *pAction = GetAction(asName);
		if(pAction==NULL)return false;

		return pAction->WasTriggerd();
	}
	
	bool cInput::WasTriggerd(int alId)
	{
		cAction *pAction = GetAction(alId);
		if(pAction==NULL)return false;

		return pAction->WasTriggerd();
	}

	//-----------------------------------------------------------------------
	
	bool cInput::BecameTriggerd(const tString& asName)
	{
		cAction *pAction = GetAction(asName);
		if(pAction==NULL)return false;

		return pAction->BecameTriggerd();
	}

	bool cInput::BecameTriggerd(int alId)
	{
		cAction *pAction = GetAction(alId);
		if(pAction==NULL)return false;

		return pAction->BecameTriggerd();
	}

	//-----------------------------------------------------------------------

	bool cInput::DoubleTriggerd(const tString& asName, float afLimit)
	{
		cAction *pAction = GetAction(asName);
		if(pAction==NULL)return false;

		return pAction->DoubleTriggerd(afLimit);
	}
	bool cInput::DoubleTriggerd(int alId, float afLimit)
	{
		cAction *pAction = GetAction(alId);
		if(pAction==NULL)return false;

		return pAction->DoubleTriggerd(afLimit);
	}

	//-----------------------------------------------------------------------

	iKeyboard* cInput::GetKeyboard()
	{
		return mpKeyboard;
	}

	//-----------------------------------------------------------------------

	iMouse* cInput::GetMouse()
	{
		return mpMouse;
	}

	//-----------------------------------------------------------------------

	void cInput::RefreshGamepads()
	{
		tGamepadListIt it = mlstGamepads.begin();
		for(; it!=mlstGamepads.end(); ++it)
			mlstInputDevices.remove(*it);
		STLDeleteAll(mlstGamepads);

		for(int i=0; i<mpLowLevelInput->GetPluggedGamepadNum(); ++i)
		{
			iGamepad* pGamepad = mpLowLevelInput->CreateGamepad(i);

			if(pGamepad->GetGamepadName() == "")
			{
				hplDelete(pGamepad);
			}
			else
			{
				mlstGamepads.push_back(pGamepad);
				mlstInputDevices.push_back(pGamepad);
			}
		}
	}

	int cInput::GetGamepadNum()
	{
		return (int)mlstGamepads.size();
	}

	iGamepad* cInput::GetGamepad(int alIdx)
	{
		tGamepadListIt it = mlstGamepads.begin();
		for(size_t i=0; it!=mlstGamepads.end(); ++i, ++it)
		{
			if(i==alIdx)
				return *it;
		}

		return NULL;
	}
	
	//-----------------------------------------------------------------------

	cAction* cInput::GetAction(const tString& asName)
	{
		tActionMapIt it = m_mapActions.find(asName);
		if(it==m_mapActions.end())return NULL;

		return it->second;
	}

	cAction* cInput::GetAction(int alId)
	{
		tActionIdMapIt it = m_mapActionIds.find(alId);
		if(it==m_mapActionIds.end()) return NULL;

		return it->second;
	}

	//-----------------------------------------------------------------------

	void cInput::DestroyAction(cAction *apAction)
	{
		for(tActionMapIt it = m_mapActions.begin(); it!= m_mapActions.end();++it)
		{
			if(it->second == apAction)
			{
				m_mapActions.erase(it);
				break;
			}
		}

		for(tActionIdMapIt it = m_mapActionIds.begin(); it!= m_mapActionIds.end();++it)
		{
			if(it->second == apAction)
			{
				m_mapActionIds.erase(it);
				break;
			}
		}

		if(apAction) hplDelete(apAction);
	}

	//-----------------------------------------------------------------------

	bool cInput::CheckForInput()
	{
		//////////////////////
		//Keyboard
		for(int i=0; i< eKey_LastEnum; ++i)
		{
			if(mpKeyboard->KeyIsDown((eKey)i)) return true;
		}

		//////////////////////
		//Mouse
		for(int i=0; i< eMouseButton_LastEnum; ++i)
		{
			if(mpMouse->ButtonIsDown((eMouseButton)i)) return true;
		}
		
		return false;
	}

	//-----------------------------------------------------------------------

	void cInput::ResetActionsToCurrentState()
	{
		for(tActionMapIt it = m_mapActions.begin(); it!= m_mapActions.end();++it)
		{
			cAction *pAction = it->second;
			pAction->ResetToCurrentState();			
		}

	}

	//-----------------------------------------------------------------------
	
	iSubAction* cInput::InputToSubAction()
	{
		iSubAction *pSubAction=NULL;
		
		//////////////////////
		//Keyboard
		for(int i=0; i< eKey_LastEnum; ++i)
		{
			if(mpKeyboard->KeyIsDown((eKey)i))
			{
				pSubAction = hplNew( cActionKeyboard, (this,(eKey)i));
				break;
			}
		}

		//////////////////////
		//Mouse
		if(pSubAction==NULL)
		{
			for(int i=0; i< eMouseButton_LastEnum; ++i)
			{
				if(mpMouse->ButtonIsDown((eMouseButton)i))
				{
					pSubAction = hplNew( cActionMouseButton, (this,(eMouseButton)i));					
					break;
				}
			}
		}

		return pSubAction;
	}
	
	//-----------------------------------------------------------------------

	void cInput::AppDeviceWasPlugged()
	{
		RefreshGamepads();
	}

	void cInput::AppDeviceWasRemoved()
	{
		RefreshGamepads();
	}

	//-----------------------------------------------------------------------


    bool cInput::isQuitMessagePosted()
    {
        return mpLowLevelInput->isQuitMessagePosted();
    }

    void cInput::resetQuitMessagePosted()
    {
        mpLowLevelInput->resetQuitMessagePosted();
    }

    //-----------------------------------------------------------------------

}
