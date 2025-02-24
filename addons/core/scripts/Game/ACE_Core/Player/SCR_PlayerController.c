//------------------------------------------------------------------------------------------------
modded class SCR_PlayerController : PlayerController
{
	//------------------------------------------------------------------------------------------------
	//! Request deletion of unreplicated entity from all machines
	//! Called from local player
	void ACE_RequestDeleteEntity(IEntity entity)
	{
		Rpc(RpcAsk_ACE_DeleteEntityByBits, ACE_EntityIdHelper.ToInt(entity.GetID()));
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ACE_DeleteEntityByBits(array<int> bits)
	{
		ACE_LoadtimeEntityManager manager = ACE_LoadtimeEntityManager.GetInstance();
		if (!manager)
			return;
		
		manager.DeleteEntitiesByIdGlobal({EntityID.FromInt(bits[0], bits[1])});
	}
	
	//------------------------------------------------------------------------------------------------
	void ACE_RequestAnimateWithHelperCompartment(ACE_EAnimationHelperID helperID)
	{
		Rpc(RpcAsk_ACE_AnimateWithHelperCompartment, helperID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ACE_AnimateWithHelperCompartment(ACE_EAnimationHelperID helperID)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(GetControlledEntity());
		// Skip if no character or character inside something else
		if (!char || char.GetParent())
			return;
		
		ACE_AnimationTools.AnimateWithHelperCompartment(helperID, char);
	}
}
