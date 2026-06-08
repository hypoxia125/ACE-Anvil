class ACE_Medical_Defibrillation_ECGWaveformGeneratorComponentClass : ScriptComponentClass
{
}

class ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent : ScriptComponent
{
	// Waveform state
	protected float m_fSampleRate;
	protected float m_fDt;
	protected float m_fCurrentTime;
	
	// Noise parameters
	protected float m_fNoiseLevel;
	protected float m_fBaseline;
	protected float m_fBaselineDrift;
	
	protected ACE_Medical_VitalsComponent m_pVitalsComponent;
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		m_fSampleRate = 250.0;
		m_fDt = 1.0 / m_fSampleRate;
		m_fCurrentTime = 0.0;
		
		m_fNoiseLevel = 0.005;
		m_fBaseline = 0.0;
		m_fBaselineDrift = 0.01;
		
		// Get the vials component
		m_pVitalsComponent = ACE_Medical_VitalsComponent.Cast(GetOwner().FindComponent(ACE_Medical_VitalsComponent));
		if (!m_pVitalsComponent)
			Print("No vitals component for ECG Component!", level: LogLevel.FATAL);
		
		Print("ECG Waveform Generator initialized");
	}
	
	//------------------------------------------------------------------------------------------------
	ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent GetECGGeneratorComponent()
	{
		ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent component = ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent.Cast(GetOwner().FindComponent(ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent));
		if (!component)
			return null;
		
		return component;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetHeartRate()
	{
		return m_pVitalsComponent.GetHeartRate();
	}
	
	//------------------------------------------------------------------------------------------------
	ACE_Medical_Defibrillation_ECardiacRhythm GetCardiacRhythm()
	{
		return m_pVitalsComponent.GetCardiacRhythm();
	}
	
	//------------------------------------------------------------------------------------------------
	protected float FMSine(float amp, float centerFreq, float modAmp, float modFreq, float t)
	{
		float freq = centerFreq + modAmp * Math.Sin(2 * Math.PI * modFreq * t);
		return amp * Math.Sin(2 * Math.PI * freq * t);
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GaussianNoise()
	{
		float u1 = Math.RandomFloat(0, 1);
		float u2 = Math.RandomFloat(0, 1);
		return Math.Sqrt(-2.0 * Math.Log(u1)) * Math.Cos(2 * Math.PI * u2);
	}
	
	//------------------------------------------------------------------------------------------------
	protected float ApplyBaselineWander(float sample)
	{
		m_fBaseline += GaussianNoise() * m_fBaselineDrift;
		m_fBaseline = Math.Clamp(m_fBaseline, -0.08, 0.08);
		
		return sample + m_fBaseline;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GenerateSinusSample()
	{
		float iBPM = GetHeartRate();
		float beatPeriod = 60.0 / iBPM;
		float beatPosition = Math.Mod(m_fCurrentTime, beatPeriod) / beatPeriod;
		
		// P wave
		float pAmp = 0.2;
		float pDuration = 0.10;
		float pStartTime = 0.08;
		float pCenterTime = pStartTime + (pDuration / 2);
		
		float pStart = pStartTime / beatPeriod;
		float pEnd = (pStartTime + pDuration) / beatPeriod;
		float pCenter = pCenterTime / beatPeriod;
		
		float pWave = 0;
		if (beatPosition >= pStart && beatPosition <= pEnd)
		{
			float pWidthNorm = pDuration / beatPeriod;
			pWave = pAmp * Math.Pow(Math.E, -Math.Pow(beatPosition - pCenter, 2) / (2 * Math.Pow(pWidthNorm / 4, 2)));
		}
		
		// PR interval and QRS
		float prInterval = 0.12;
		float qrsStartTime = pStartTime + prInterval;
			
		float qAmp = 0;
		float rAmp = 1.0;
		float sAmp = -0.3;
		float qrsDuration = 0.10;
			
		float qrsStart = qrsStartTime / beatPeriod;
		float qrsEnd = (qrsStartTime + qrsDuration) / beatPeriod;
			
		float qrs = 0;
		if (beatPosition >= qrsStart && beatPosition <= qrsEnd)
		{
			float qrsPos = (beatPosition - qrsStart) / (qrsDuration / beatPeriod);
				
			if (qrsPos < 0.20)
			{
				qrs = qAmp * (qrsPos / 0.20);
			}
			else if (qrsPos < 0.60)
			{
				float rPos = (qrsPos - 0.20) / 0.40;
				qrs = rAmp * Math.Sin(rPos * Math.PI);
			}
			else
			{
				float sPos = (qrsPos - 0.60) / 0.40;
				qrs = sAmp * Math.Sin(sPos * Math.PI);
			}
		}
			
		// T-wave
		float tAmp = 0.15;
		float tDuration = 0.18;
		float tStartTime = qrsStartTime + qrsDuration + 0.02;
		float tCenterTime = tStartTime + (tDuration / 2);
		
		float tStart = tStartTime / beatPeriod;
		float tEnd = (tStartTime + tDuration) / beatPeriod;
		float tCenter = tCenterTime / beatPeriod;
		
		float tWave = 0;
		if (beatPosition >= tStart && beatPosition <= tEnd)
		{
			float tWidthNorm = tDuration / beatPeriod;
			tWave = tAmp * Math.Pow(Math.E, -Math.Pow(beatPosition - tCenter, 2) / (2 * Math.Pow(tWidthNorm / 4, 2)));
		}
		
		float sample = pWave + qrs + tWave;
		
		sample += GaussianNoise() * m_fNoiseLevel;
			
		return sample;
	}

	//------------------------------------------------------------------------------------------------
	protected float GenerateVFibSample()
	{
		float t = m_fCurrentTime;
		
		float slow = FMSine(0.3, 4.0, 0.5, 0.7, t);
		
		float medium = FMSine(0.2, 7.5, 1.0, 1.2, t);
		// medium += 0.2 * np.sin(np.sin(0.5 * t))
		
		float fast = 0.1 * GaussianNoise();
		
		float ampMod = 0.7 + 0.3 * Math.Sin(t * 2.5);
		
		float sample = (slow + medium + fast) * ampMod;
		
		sample += GaussianNoise() * m_fNoiseLevel;
		
		return Math.Clamp(sample, -1.2, 1.2);
	}

	//------------------------------------------------------------------------------------------------
	protected float GenerateAsystoleSample()
	{
		return GaussianNoise() * 0.01;
	}

	//------------------------------------------------------------------------------------------------
	float GenerateSample()
	{
		float sample = 0;
		
		if (GetCardiacRhythm() == ACE_Medical_Defibrillation_ECardiacRhythm.Sinus)
		{
			sample = GenerateSinusSample();
		}
		else if (GetCardiacRhythm() == ACE_Medical_Defibrillation_ECardiacRhythm.VF)
		{
			sample = GenerateVFibSample();
		}
		/*
		else if (GetCardiacRhythm() == ACE_Medical_Defibrillation_ECardiacRhythm.PEA)
		{
			sample = GenerateSinusSample();
		}
		*/
		else if (GetCardiacRhythm() == ACE_Medical_Defibrillation_ECardiacRhythm.Asystole)
		{
			sample = GenerateAsystoleSample();
		}
		
		sample = ApplyBaselineWander(sample);
		
		// Advance time
		m_fCurrentTime += m_fDt;
		
		return sample;
	}

	//------------------------------------------------------------------------------------------------
	array<float> GenerateSampleBuffer(int numSamples)
	{
		array<float> buffer = new array<float>;
		buffer.Reserve(numSamples);
		
		for (int i = 0; i < numSamples; i++)
		{
			buffer.Insert(GenerateSample());
		}
		
		return buffer;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNoiseLevel(float level)
	{
		m_fNoiseLevel = Math.Clamp(level, 0.0, 0.2);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetCurrentTime()
	{
		return m_fCurrentTime;
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_fCurrentTime = 0.0;
		m_fBaseline = 0.0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		Print("ECG Waveform Generator removed");
		super.OnDelete(owner);
	}
}