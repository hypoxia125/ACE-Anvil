class ACE_Medical_Defibrillation_UserAction_DefibMenuOpen : ScriptedUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		super.CanBeShownScript(user);
		
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		Print("Remove: Opening Menu");
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ACE_Medical_Defibrillation_DefibrillatorMenu);
	}
}