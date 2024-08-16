//------------------------------------------------------------------------------------------------
class ACE_Medical_BaseComponent2Class : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! This is a copy of ACE_Medical_BaseComponent2
//! TO DO: Delete once systems support inheritance
class ACE_Medical_BaseComponent2 : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//! Returns the type of the system that updates this component
	//! Derived classes should always override this!
	typename GetAssociatedSystemType()
	{
		return ACE_Medical_BaseSystem2;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (!GetGame().InPlayMode() || !Replication.IsServer())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register to system on init
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		ACE_Medical_BaseSystem2 system = ACE_Medical_BaseSystem2.GetInstance(GetAssociatedSystemType());
		if (system)
			system.Register(owner);
		
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));
		if (damageManager)
			damageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregister from system when killed
	protected void OnDamageStateChanged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		ACE_Medical_BaseSystem2 system = ACE_Medical_BaseSystem2.GetInstance(GetAssociatedSystemType());
		if (system)
			system.Unregister(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregister from system in destructor is too late, hence we do it in SCR_EditableCharacterComponent.OnDelete instead
	void ~ACE_Medical_BaseComponent2();
}