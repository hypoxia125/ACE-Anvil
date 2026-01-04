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
		
		return false;
	}
}
#endif