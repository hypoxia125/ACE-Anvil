#ifdef ENABLE_DIAG
class ACE_Medical_Defibrillation_DiagTools
{
	static bool GetDiagTargetDefib(out IEntity target, out ACE_Medical_Defibrillation_DefibComponent defibComponent)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return false;
		
		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return false;
		
		target = IEntity.Cast(camera.GetCursorTarget());
		if (!target)
			return false;
		
		defibComponent = ACE_Medical_Defibrillation_DefibComponent.Cast(target.FindComponent(ACE_Medical_Defibrillation_DefibComponent));
		if (!defibComponent)
			return false;
		
		return true;
	}
	
	static bool GetDiagNearestDefib(out IEntity target, out ACE_Medical_Defibrillation_DefibComponent defibComponent, float distance = 3)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return false;
		
		vector playerPos = player.GetOrigin();
		
		ACE_Medical_Defibrillation_QueryNearestDefib query = new ACE_Medical_Defibrillation_QueryNearestDefib(distance);
		target = query.GetEntity(playerPos);
		if (!target)
			return false;
		
		defibComponent = ACE_Medical_Defibrillation_DefibComponent.Cast(target.FindComponent(ACE_Medical_Defibrillation_DefibComponent));
		if (!defibComponent)
			return false;
		
		return true;
	}
}
#endif