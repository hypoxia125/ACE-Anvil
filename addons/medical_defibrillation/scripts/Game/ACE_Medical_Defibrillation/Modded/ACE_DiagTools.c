modded class ACE_DiagTools
{
	//------------------------------------------------------------------------------------------------
	static float GetReviveChanceShockBonus(IEntity target)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(target);
		if (!char)
			return -1;
		
		ACE_Medical_CharacterContext context = new ACE_Medical_CharacterContext(char);
		ACE_Medical_ReviveTransition transition = new ACE_Medical_ReviveTransition(ACE_Medical_EVitalStateID.ANY, ACE_Medical_EVitalStateID.ANY);
		
		return transition.ComputeReviveChanceShockBonus(context);
	}
	
	//------------------------------------------------------------------------------------------------
	static float GetReviveChance(IEntity target)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(target);
		if (!char)
			return -1;
		
		ACE_Medical_CharacterContext context = new ACE_Medical_CharacterContext(char);
		ACE_Medical_ReviveTransition transition = new ACE_Medical_ReviveTransition(ACE_Medical_EVitalStateID.ANY, ACE_Medical_EVitalStateID.ANY);
		
		return transition.ComputeReviveChance(context);
	}
}