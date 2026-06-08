//------------------------------------------------------------------------------------------------
// ACE_Medical_ECGMonitorDisplay : SCR_InfoDisplayExtended
// Handles ONLY drawing and display logic - references the component
//------------------------------------------------------------------------------------------------
class ACE_Medical_ECGMonitorDisplay : SCR_InfoDisplayExtended
{
	protected Widget m_wLayout;
	protected Widget m_wCanvasContainer;
	protected Widget m_wTextContainer;
	protected CanvasWidget m_wCanvas;
	protected vector m_vCanvasCenter;
	
	// Drawing commands
	protected ref LineDrawCommand m_WaveformLine = new LineDrawCommand();
	protected ref LineDrawCommand m_GridHorizontal = new LineDrawCommand();
	protected ref LineDrawCommand m_GridVertical = new LineDrawCommand();
	protected ref TextWidget m_HeartRateText;
	protected ref array<ref CanvasWidgetCommand> m_aDrawCommands;
	
	// Display constants
	protected const float DISPLAY_WIDTH = 512;
	protected const float DISPLAY_HEIGHT = 256;
	protected const float LINE_WIDTH = 2.0;
	protected const float GRID_SMALL_SIZE = 16;
	protected const float GRID_LARGE_SIZE = 80;
	protected const float TIME_WINDOW = 6.0;
	protected const float AMPLITUDE_SCALE = 80.0;
	
	// Reference to the waveform generator component
	protected ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent m_WaveformGenerator;
	protected ref array<float> m_WaveformBuffer;
	protected int m_iBufferSize;
	
	// UI timing
	protected float m_fLastUpdateTime;
	protected float m_fUpdateInterval = 0.033;
	protected float m_fDPIScale;
	protected float m_fDisplayWidthScaled;
	protected float m_fDisplayHeightScaled;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wLayout = m_wRoot.FindWidget("Layout");
		m_wCanvasContainer = m_wLayout.FindWidget("CanvasContainer");
		m_wTextContainer = m_wLayout.FindWidget("TextContainer");
		m_wCanvas = CanvasWidget.Cast(m_wCanvasContainer.FindWidget("Canvas"));
		m_HeartRateText = TextWidget.Cast(m_wTextContainer.FindWidget("HeartRateValue"));
		
		// Initialize drawing commands
		m_WaveformLine.m_iColor = Color.GREEN;
		m_WaveformLine.m_fWidth = LINE_WIDTH;
		m_GridHorizontal.m_iColor = Color.FromRGBA(255, 255, 255, 80);
		m_GridVertical.m_iColor = Color.FromRGBA(255, 255, 255, 80);
		m_GridHorizontal.m_fWidth = 1;
		m_GridVertical.m_fWidth = 1;
		
		m_aDrawCommands = {m_GridHorizontal, m_GridVertical, m_WaveformLine};
		
		m_WaveformGenerator = GetECGWaveformGeneratorComponent();
		if (m_WaveformGenerator)
		{
			// Setup display buffer
			m_iBufferSize = (int)(TIME_WINDOW * 250.0);
			m_WaveformBuffer = new array<float>;
			m_WaveformBuffer.Reserve(m_iBufferSize);
			
			// Pre-fill buffer
			array<float> initialSamples = m_WaveformGenerator.GenerateSampleBuffer(m_iBufferSize);
			for (int i = 0; i < initialSamples.Count(); i++)
			{
				m_WaveformBuffer.Insert(initialSamples.Get(i));
			}
			
			UpdateUIText();
		}
		
		GetGame().GetInputManager().AddActionListener("ACE_Medical_ECGToggle", EActionTrigger.DOWN, ToggleDisplay);
	}
	
	//------------------------------------------------------------------------------------------------
	ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent GetECGWaveformGeneratorComponent()
	{
		ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent genComp = ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent.Cast(GetOwnerEntity().FindComponent(ACE_Medical_Defibrillation_ECGWaveformGeneratorComponent));
		if (!genComp)
			return null;
		
		return genComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDPIScale(float scale)
	{
		m_fDPIScale = scale;
		m_fDisplayWidthScaled = m_fDPIScale * DISPLAY_WIDTH;
		m_fDisplayHeightScaled = m_fDPIScale * DISPLAY_HEIGHT;
		m_vCanvasCenter = m_fDPIScale * 0.5 * FrameSlot.GetSize(m_wLayout);
		m_WaveformLine.m_fWidth = m_fDPIScale * LINE_WIDTH;
		UpdateGrid();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateGrid()
	{
		m_GridHorizontal.m_Vertices = {};
		m_GridVertical.m_Vertices = {};
		
		float startX = m_vCanvasCenter[0] - (m_fDisplayWidthScaled / 2);
		float endX = m_vCanvasCenter[0] + (m_fDisplayWidthScaled / 2);
		float startY = m_vCanvasCenter[1] - (m_fDisplayHeightScaled / 2);
		float endY = m_vCanvasCenter[1] + (m_fDisplayHeightScaled / 2);
		
		float gridSmallScaled = m_fDPIScale * GRID_SMALL_SIZE;
		float gridLargeScaled = m_fDPIScale * GRID_LARGE_SIZE;
		
		// Horizontal grid lines
		for (float y = startY; y <= endY + gridLargeScaled; y += gridLargeScaled)
		{
			m_GridHorizontal.m_Vertices.Insert(startX);
			m_GridHorizontal.m_Vertices.Insert(y);
			m_GridHorizontal.m_Vertices.Insert(endX);
			m_GridHorizontal.m_Vertices.Insert(y);
			
			for (int sub = 1; sub < 5; sub++)
			{
				float subY = y + (sub * gridSmallScaled);
				if (subY <= endY + gridSmallScaled)
				{
					m_GridHorizontal.m_Vertices.Insert(startX);
					m_GridHorizontal.m_Vertices.Insert(subY);
					m_GridHorizontal.m_Vertices.Insert(endX);
					m_GridHorizontal.m_Vertices.Insert(subY);
				}
			}
		}
		
		// Vertical grid lines
		for (float x = startX; x <= endX + gridLargeScaled; x += gridLargeScaled)
		{
			m_GridVertical.m_Vertices.Insert(x);
			m_GridVertical.m_Vertices.Insert(startY);
			m_GridVertical.m_Vertices.Insert(x);
			m_GridVertical.m_Vertices.Insert(endY);
			
			for (int sub = 1; sub < 5; sub++)
			{
				float subX = x + (sub * gridSmallScaled);
				if (subX <= endX + gridSmallScaled)
				{
					m_GridVertical.m_Vertices.Insert(subX);
					m_GridVertical.m_Vertices.Insert(startY);
					m_GridVertical.m_Vertices.Insert(subX);
					m_GridVertical.m_Vertices.Insert(endY);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWaveformBuffer()
	{
		if (!m_WaveformGenerator)
			return;
		
		while (m_WaveformBuffer.Count() < m_iBufferSize)
		{
			float sample = m_WaveformGenerator.GenerateSample();
			m_WaveformBuffer.Insert(sample);
		}
		
		while (m_WaveformBuffer.Count() > m_iBufferSize)
		{
			m_WaveformBuffer.Remove(0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DrawWaveform()
	{
		if (m_WaveformBuffer.Count() < 2)
			return;
		
		float startX = m_vCanvasCenter[0] - (m_fDisplayWidthScaled / 2);
		float endX = m_vCanvasCenter[0] + (m_fDisplayWidthScaled / 2);
		float width = endX - startX;
		
		m_WaveformLine.m_Vertices = {};
		
		float timeScale = width / TIME_WINDOW;
		float baselineY = m_vCanvasCenter[1];
		float verticalScale = AMPLITUDE_SCALE * m_fDPIScale;
		
		int numSamples = m_WaveformBuffer.Count();
		
		for (int i = 0; i < numSamples - 1; i++)
		{
			float timePos = (i / 250.0);
			float x1 = startX + (timePos * timeScale);
			float x2 = startX + ((i + 1) / 250.0 * timeScale);
			
			if (x2 < startX || x1 > endX)
				continue;
			
			float sample1 = m_WaveformBuffer.Get(i);
			float sample2 = m_WaveformBuffer.Get(i + 1);
			
			float y1 = baselineY - (sample1 * verticalScale);
			float y2 = baselineY - (sample2 * verticalScale);
			
			m_WaveformLine.m_Vertices.Insert(x1);
			m_WaveformLine.m_Vertices.Insert(y1);
			m_WaveformLine.m_Vertices.Insert(x2);
			m_WaveformLine.m_Vertices.Insert(y2);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateUIText()
	{
		if (!m_WaveformGenerator)
			return;
			
		if (m_HeartRateText)
		{
			int bpm = m_WaveformGenerator.GetHeartRate();
			if (bpm == 0)
				m_HeartRateText.SetText("--");
			else if (m_WaveformGenerator.GetCardiacRhythm() == ACE_Medical_Defibrillation_ECardiacRhythm.VF)
				m_HeartRateText.SetText("???");
			else
				m_HeartRateText.SetText(bpm.ToString());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		GetGame().GetInputManager().RemoveActionListener("ACE_Medical_ECGToggle", EActionTrigger.DOWN, ToggleDisplay);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!IsShown() || !m_WaveformGenerator)
			return;
		
		float dpiScale = GetGame().GetWorkspace().DPIScale(1);
		if (m_fDPIScale != dpiScale)
			UpdateDPIScale(dpiScale);
		
		float currentTime = GetGame().GetWorld().GetWorldTime();
		if (currentTime - m_fLastUpdateTime >= m_fUpdateInterval)
		{
			m_fLastUpdateTime = currentTime;
			UpdateWaveformBuffer();
			DrawWaveform();
			m_wCanvas.SetDrawCommands(m_aDrawCommands);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ToggleDisplay()
	{
		Show(!IsShown());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNoiseLevel(float level)
	{
		if (m_WaveformGenerator)
			m_WaveformGenerator.SetNoiseLevel(level);
	}
	
	void ResetECG()
	{
		if (!m_WaveformGenerator)
			return;
			
		m_WaveformGenerator.Reset();
		m_WaveformBuffer.Clear();
		
		array<float> newSamples = m_WaveformGenerator.GenerateSampleBuffer(m_iBufferSize);
		for (int i = 0; i < newSamples.Count(); i++)
		{
			m_WaveformBuffer.Insert(newSamples.Get(i));
		}
	}
}