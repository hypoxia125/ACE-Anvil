class ACE_Medical_Defibrillation_DefibComponentClass : ScriptComponentClass
{
}

class ACE_Medical_Defibrillation_DefibComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.ComboBox, "Defibrillator Emulation Type", "", ParamEnumArray.FromEnum(ACE_Medical_Defibrillation_EDefibEmulation), category: "General Settings")]
	protected ACE_Medical_Defibrillation_EDefibEmulation m_eDefibrillatorEmulation;
	
	protected float m_fChargeDuration = 5.5;
	protected float m_fAnalysisDuration;
	protected float m_fCPRCooldownDuration;
	protected bool m_bPlayCPRPacingBeats;
	
	[Attribute(defvalue: "false", desc: "Always sets the AED to detect shockable rhythm. Use to quickly test shock systems.", category: "DEBUG")]
	bool m_bDebugAEDAlwaysShockableRhythm;
	
	[RplProp(onRplName: "OnPatientReplicated")]
	protected int m_iPatientRplId;
	protected IEntity m_pPatient;
	
	[RplProp(onRplName: "OnDefibStateChanged")]
	protected ACE_Medical_Defibrillation_EDefibStateID m_eDefibrillatorStateID;
	
	[RplProp(onRplName: "OnDefibProgressChanged"), RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected ref ACE_Medical_Defibrillation_DefibProgressData m_pProgressData;
	
	ref ACE_Medical_Defibrillation_DefibSoundTimers m_pSoundTimers;
	
	protected bool m_bCPRBeepLoop = false;
	protected bool m_bChargedBeepLoop = false;
	
	protected ACE_Medical_Defibrillation_Settings m_pSettings;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{	
		super.EOnInit(owner);
		
		SetEventMask(owner, EntityEvent.FRAME);
		
		m_pSettings = ACE_SettingsHelperT<ACE_Medical_Defibrillation_Settings>.GetModSettings();
		
		if (m_pSettings) // m_pSettings will be null on world creation in workbench
		{
			m_fCPRCooldownDuration = m_pSettings.m_fAED_CPRCooldownDuration;
			m_bPlayCPRPacingBeats = m_pSettings.m_bAED_PlayCPRPacingBeats;
			m_fAnalysisDuration = m_pSettings.m_fAED_AnalysisDuration;
		}
		
		// Convert to milliseconds and make data
		m_pProgressData = new ACE_Medical_Defibrillation_DefibProgressData(this,
																		   m_fAnalysisDuration * 1000,
																		   m_fChargeDuration * 1000,
																		   m_fCPRCooldownDuration * 1000);
		
		// Create sound data
		m_pSoundTimers = new ACE_Medical_Defibrillation_DefibSoundTimers();
		
		// Subscribe to the InventoryItemComponent OnParentSlotChanged
		// Determines if defib is already on the ground to add it to the system
		InventoryItemComponent invComp = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (invComp)
		{
			invComp.m_OnParentSlotChangedInvoker.Insert(OnParentSlotChanged);
			if (!invComp.GetParentSlot())
			{
				ACE_Medical_Defibrillation_DefibStatesSystem system = GetDefibStatesSystem();
				if (system)
					system.Register(owner);
			}
		}
		
		// Subscribe to data change events for replication
		m_pProgressData.m_OnDataChanged.Insert(OnDefibProgressChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Temporary until a client side sound manager system is possibly implimented
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		timeSlice *= 1000;
		
		// CPR Beep
		if (m_bPlayCPRPacingBeats && m_bCPRBeepLoop)
		{
			m_pSoundTimers.m_fLastCPRPaceTimer += timeSlice;
			
			if (m_pSoundTimers.m_fLastCPRPaceTimer >= ACE_Medical_Defibrillation_ConversionHelper.BpmToMs(102))
			{
				PlaySound(ACE_Medical_Defibrillation_SharedSounds.SOUNDCPRBEEP);
				m_pSoundTimers.m_fLastCPRPaceTimer = 0;
			}
		}
		
		// Charged Sound
		if (m_bChargedBeepLoop)
		{
			const float BEEP_INTERVAL = 250;
			
			if (m_pSoundTimers.m_fChargedBeepTimer >= BEEP_INTERVAL)
			{
				if (Math.Mod(m_pSoundTimers.m_iChargedBeepPhase, 2) == 0)
				{
					PlaySound(ACE_Medical_Defibrillation_SharedSounds.SOUNDCHARGEDBEEPLOW);
				}
				else
				{
					PlaySound(ACE_Medical_Defibrillation_SharedSounds.SOUNDCHARGEDBEEPHIGH);
				}
				
				// Advance to next phase and reset timer
				m_pSoundTimers.m_iChargedBeepPhase++;
				m_pSoundTimers.m_fChargedBeepTimer = 0;
			}
			
			m_pSoundTimers.m_fChargedBeepTimer += timeSlice;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SoundComponent GetSoundComponent()
	{
		SoundComponent component = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!component)
			return null;
		
		return component;
	}
	
	//------------------------------------------------------------------------------------------------
	bool DefibrillatorPatientInRange()
	{
		if (!m_pPatient)
			return false;
		
		return (vector.Distance(GetOwner().GetOrigin(), m_pPatient.GetOrigin())) <= m_pSettings.m_fDefibrillatorConnectionRange;
	}
	
	//------------------------------------------------------------------------------------------------
	ACE_Medical_Defibrillation_DefibStatesSystem GetDefibStatesSystem()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		ACE_Medical_Defibrillation_DefibStatesSystem system = ACE_Medical_Defibrillation_DefibStatesSystem.Cast(world.FindSystem(ACE_Medical_Defibrillation_DefibStatesSystem));
		if (!system)
			return null;
		
		return system;
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		SetPatient(null);
		
		m_pSettings = ACE_SettingsHelperT<ACE_Medical_Defibrillation_Settings>.GetModSettings();
		
		// Convert to milliseconds and make data
		m_pProgressData = new ACE_Medical_Defibrillation_DefibProgressData(this,
																		   m_fAnalysisDuration * 1000,
																		   m_fChargeDuration * 1000,
																		   m_fCPRCooldownDuration * 1000);
		
		// Create sound data
		m_pSoundTimers = new ACE_Medical_Defibrillation_DefibSoundTimers();
		m_eDefibrillatorStateID = ACE_Medical_Defibrillation_EDefibStateID.DISCONNECTED;
	}
	
	//------------------------------------------------------------------------------------------------
	ACE_Medical_Defibrillation_EDefibEmulation GetDefibrillatorEmulation()
	{
		return m_eDefibrillatorEmulation;
	}
	
	//------------------------------------------------------------------------------------------------
	ACE_Medical_Defibrillation_EDefibStateID GetDefibStateID()
	{
		return m_eDefibrillatorStateID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefibStateID(ACE_Medical_Defibrillation_EDefibStateID state)
	{
		m_eDefibrillatorStateID = state;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	ACE_Medical_Defibrillation_DefibProgressData GetDefibProgressData()
	{
		return m_pProgressData;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetPatient(IEntity patient)
	{
		if (!patient)
		{
			m_iPatientRplId = ACE_Medical_Defibrillation_ReplicationHelper.GetRplIdByEntity(null);
			m_pPatient = null;
			Replication.BumpMe();
			return false;
		}
		
		ACE_Medical_VitalsComponent component = ACE_Medical_VitalsComponent.Cast(patient.FindComponent(ACE_Medical_VitalsComponent));
		if (!component)
			return false;
		
		m_iPatientRplId = ACE_Medical_Defibrillation_ReplicationHelper.GetRplIdByEntity(patient);
		m_pPatient = patient;
		Replication.BumpMe();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetPatient()
	{
		m_iPatientRplId = -1;
		m_pPatient = null;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetPatient()
	{
		return m_pPatient;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsShockableRhythm()
	{
		ACE_Medical_VitalsComponent vitals = ACE_Medical_VitalsComponent.Cast(m_pPatient.FindComponent(ACE_Medical_VitalsComponent));
		if (!vitals)
			return false;
		
		array<ACE_Medical_Defibrillation_ECardiacRhythm> shockableRhythms = { ACE_Medical_Defibrillation_ECardiacRhythm.VF };
		if (shockableRhythms.Contains(vitals.GetCardiacRhythm()))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ShockPatient()
	{
	    if (!m_pPatient)
	        return false;
	    
	    ACE_Medical_VitalsComponent vitals = ACE_Medical_VitalsComponent.Cast(m_pPatient.FindComponent(ACE_Medical_VitalsComponent));
	    if (!vitals)
	        return false;
	    
	    // Calculate shock success chance
	    float shockSuccessChance = ACE_Medical_Defibrillation_CalculationsHelper.CalculateTotalShockChance(vitals, m_pSettings);
	    
	    // Roll for shock success
	    float randomRoll = Math.RandomFloat01();
	    bool shockSuccessful = randomRoll < shockSuccessChance;
	    
	    // Only increment shock count if shock was successful
	    if (shockSuccessful)
	    {
	        vitals.ModifyShocksDelivered(1);
			vitals.ResetTimeSinceLastShock();
	    }
		
		PlaySoundOnPatient(ACE_Medical_Defibrillation_SharedSounds.SOUNDSHOCKTHUMP);	    
	    SetDefibStateID(ACE_Medical_Defibrillation_EDefibStateID.CONNECTED);
	    
	    float cprCooldown = m_pProgressData.GetDuration(ACE_Medical_Defibrillation_EDefibProgressCategory.CPRCooldown);
	    m_pProgressData.SetTimer(ACE_Medical_Defibrillation_EDefibProgressCategory.CPRCooldown, cprCooldown);
	    
	    return shockSuccessful;
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnDefibProgressChanged()
	{
		Replication.BumpMe();
	}
	
	
	//------------------------------------------------------------------------------------------------
	private void OnParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		ACE_Medical_Defibrillation_DefibStatesSystem system = GetDefibStatesSystem();
		if (!system)
			return;
		
		if (!newSlot)
		{
			system.Register(GetOwner());
		}
		else
		{
			system.Unregister(GetOwner());
			Reset();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnDefibStateChanged()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPatientReplicated()
	{
		GetGame().GetCallqueue().Call(OnPatientReplicatedDelayed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPatientReplicatedDelayed()
	{
		m_pPatient = ACE_Medical_Defibrillation_ReplicationHelper.GetEntityByRplId(m_iPatientRplId);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCPRBeepLoop(bool state = false)
	{
		m_bCPRBeepLoop = state;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetChargedBeepLoop(bool state = false)
	{
		m_bChargedBeepLoop = state;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySound(string soundName, bool terminatePrevious = false, bool isLoop = false)
	{
		if (!Replication.IsServer())
			return;
		
		RPC_PlaySound(soundName, terminatePrevious, isLoop);
		Rpc(RPC_PlaySound, soundName, terminatePrevious, isLoop);
	}
	
	//------------------------------------------------------------------------------------------------
	void TerminateSound(bool terminateAll = false)
	{
		if (!Replication.IsServer())
			return;
		
		RPC_TerminateAllSounds();
		Rpc(RPC_TerminateAllSounds);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_PlaySound(string soundName, bool terminatePrevious, bool isLoop)
	{		
		if (isLoop)
			return;
		
		if (terminatePrevious)
			TerminateSound();
		
		SoundComponent sndComp = GetSoundComponent();
		if (sndComp)
			sndComp.SoundEvent(soundName);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_TerminateAllSounds()
	{
		SoundComponent sndComp = GetSoundComponent();
		if (!sndComp)
			return;
		
		sndComp.TerminateAll();
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySoundOnPatient(string soundName)
	{
		if (!Replication.IsServer())
			return;
		
		RPC_PlaySoundOnPatient(ACE_Medical_Defibrillation_SharedSounds.SOUNDSHOCKTHUMP);
		Rpc(RPC_PlaySoundOnPatient, ACE_Medical_Defibrillation_SharedSounds.SOUNDSHOCKTHUMP);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_PlaySoundOnPatient(string soundName)
	{
		IEntity patient = GetPatient();
		if (!patient)
			return;
		
		CharacterSoundComponent sndComponent = CharacterSoundComponent.Cast(patient.FindComponent(CharacterSoundComponent));
		if (!sndComponent)
			return;
		
		sndComponent.SoundEvent(soundName);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetDebugAlwaysShockableRhythm()
	{
		return m_bDebugAEDAlwaysShockableRhythm;
	}
}