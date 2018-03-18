
// VarioPreferenceDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "wconsole.h"
#include "VarioPreferenceDlg.h"
#include "afxdialogex.h"

#include <strsafe.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Parameter tables
//

struct GliderInfo
{
	GliderType	type;
	LPCTSTR		name;
};

struct TimeZoneInfo
{
	int			offset;
	LPCTSTR		disp;
};

struct VolumeInfo
{
	int			max;
	LPCTSTR		disp;
};

struct SentenceInfo
{
	VarioSentence	type;
	LPCTSTR			name;
};

static GliderInfo _GliderTable[] =
{
	{ GTYPE_UNKNOWNM, _T("Unset")				},
	{ GTYPE_PARAGLIDER, _T("Paraglider")		},
	{ GTYPE_HG_FLEX, _T("Hanglider Flex")		},
	{ GTYPE_HG_RIGID, _T("Hanglider Rigid")		},
	{ GTYPE_SAILPLANE, _T("Sailplane")			},
	{ GTYPE_PPG_FOOT, _T("PParaglider Foot")	},
	{ GTYPE_PPG_WHEEL, _T("PParaglider Wheel")	},
};

static TimeZoneInfo _TimezoneTable[] =
{
	{ -11, _T("GMT-11")	},
	{ -10, _T("GMT-10")	},
	{  -9, _T("GMT-9")	},
	{  -8, _T("GMT-8")	},
	{  -7, _T("GMT-7")	},
	{  -6, _T("GMT-6")	},
	{  -5, _T("GMT-5")	},
	{  -4, _T("GMT-4")	},
	{  -3, _T("GMT-3")	},
	{  -2, _T("GMT-2")	},
	{  -1, _T("GMT-1")	},
	{  +0, _T("GMT+0")	},
	{  +1, _T("GMT+1")	},
	{  +2, _T("GMT+2")	},
	{  +3, _T("GMT+3")	},
	{  +4, _T("GMT+4")	},
	{  +5, _T("GMT+5")	},
	{  +6, _T("GMT+6")	},
	{  +7, _T("GMT+7")	},
	{  +8, _T("GMT+8")	},
	{  +9, _T("GMT+9")	},
	{ +10, _T("GMT+10") },
	{ +11, _T("GMT+11") },
	{ +12, _T("GMT+12") },
	{ +13, _T("GMT+13") },
	{ +14, _T("GMT+14")	},
};

static VolumeInfo _VolumeTable[] =
{
	{   0,	_T("Mute") },
	{   5,	_T("Medium") },
	{ 100,	_T("Loud") },
};

static SentenceInfo _SentenceTable[] =
{
	{ VSENTENCE_LK8,	_T("LK8")	},
	{ VSENTENCE_LXNAV,	_T("LxNav")	},
};

static VarioTone _ToneTable[TONE_TABLE_COUNT] =
{
	{ -10.0f,	200,	200,	100 },
	{ -3.0f,	293,	200,	100 },
	{ -2.0f,	369,	200,	100 },
	{ -1.0f,	440,	200,	100 },
	{ 0.09f,	400,	600,	50	},
	{ 0.10f,	400,	600,	50	},
	{ 1.98f,	499,	552,	50	},
	{ 3.14f,	868,	347,	50	},
	{ 4.57f,	1084,	262,	50	},
	{ 6.28f,	1354,	185,	50	},
	{ 8.15f,	1593,	168,	50	},
	{ 10.00f,	1800,	150,	50	},
};


static int GetGliderType(GliderType type)
{
	for (int i = 0; i < sizeof(_GliderTable) / sizeof(_GliderTable[0]); i++)
	{
		if (_GliderTable[i].type == type)
			return i;
	}

	return 0;
}

static int GetTimeZone(int offset)
{
	int default = 0;

	for (int i = 0; i < sizeof(_TimezoneTable) / sizeof(_TimezoneTable[0]); i++)
	{
		if (_TimezoneTable[i].offset == offset)
			return i;

		if (_TimezoneTable[i].offset == 0)
			default = i;
	}

	return default;
}

static int GetVarioSentence(VarioSentence sentence)
{
	for (int i = 0; i < sizeof(_SentenceTable) / sizeof(_SentenceTable[0]); i++)
	{
		if (_SentenceTable[i].type == sentence)
			return i;
	}

	return 0;
}

static int GetVolume(int vol)
{
	for (int i = 0; i < sizeof(_VolumeTable) / sizeof(_VolumeTable[0]); i++)
	{
		if (vol <= _VolumeTable[i].max)
			return i;
	}

	return 0;
}



static WORD PStrToCode(CString &str)
{
	WORD code = 0;

	if (str.GetLength() == 2)
		code = ((WORD)str.GetAt(0) << 8) + (WORD)str.GetAt(1);

	return code;
}

static CString PCodeToStr(WORD code)
{
	CString str;

	str += (CHAR)(code >> 8);
	str += (CHAR)(code & 0xFF);

	return str;
}



// CVarioPreferenceDlg ��ȭ ����

CVarioPreferenceDlg::CVarioPreferenceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PREFERENCE_DIALOG, pParent)
	, m_nGliderType(0)
	, m_strGliderManufacture(_T(""))
	, m_strGliderModel(_T(""))
	, m_nLoggerTakeoffSpeed(10)
	, m_nLOggerLandingTimeout(30000)
	, m_nLoggerLoggingInterval(1000)
	, m_strPilotName(_T(""))
	, m_nTimezone(0)
	, m_bLoggerEnable(TRUE)
	, m_fVarioClimbThreshold(0.2f)
	, m_fVarioSinkThreshold(-3.0f)
	, m_fVarioSensitivity(0.12f)
	, m_nVarioSentence(0)
	, m_bVarioUseBaroOnly(TRUE)
	, m_fKalmanVarZmeas(400.0f)
	, m_fKalmanVarZaccel(1000.0f)
	, m_fKalmanVarAccelbias(1.0f)
	, m_nVolumeVario(0)
	, m_strVolumeVario(_T("Mute"))
	, m_nVolumeEffect(0)
	, m_strVolumeEffect(_T("Mute"))
	, m_fThresholdLowBattery(3.2f)
	, m_fThresholdShutdownHoldtime(2000)
	, m_fThresholdAutoPoweroffVario(600)
	, m_fThresholdAutoPoweroffUMS(600)
	, m_fCalDataAccelX(0.0f)
	, m_fCalDataAccelY(0.0f)
	, m_fCalDataAccelZ(0.0f)
	, m_fCalDataGyroX(0.0f)
	, m_fCalDataGyroY(0.0f)
	, m_fCalDataGyroZ(0.0f)
	, m_nTimerID(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// GliderInfo
	m_VarioParams.Glider_Type = GTYPE_PARAGLIDER;
	m_VarioParams.Glider_Manufacture = _T("");
	m_VarioParams.Glider_Model = _T("");
	// IGC-Logger
	m_VarioParams.Logger_Enable = TRUE;
	m_VarioParams.Logger_TakeoffSpeed = 10;
	m_VarioParams.Logger_LandingTimeout = 30000;
	m_VarioParams.Logger_LoggingInterval = 1000;
	m_VarioParams.Logger_PilotName = _T("");
	m_VarioParams.Logger_Timezone = 9;
	// Vario Settings
	m_VarioParams.Vario_ClimbThreshold = 0.2f;
	m_VarioParams.Vario_SinkThreshold = -3.0f;
	m_VarioParams.Vario_Sensitivity = 0.1f;
	m_VarioParams.Vario_Sentece = VSENTENCE_LK8;
	m_VarioParams.Vario_BaroOnly = TRUE;
	// Volume Settings
	m_VarioParams.Volume_Vario = 98;
	m_VarioParams.Volume_Effect = 5;
	// Threshold Settings
	m_VarioParams.Threshold_LowBattery = 3.2f;
	m_VarioParams.Threshold_ShutdownHoldtime = 1000;
	m_VarioParams.Threshold_AutoShutdownVario = 600000;
	m_VarioParams.Threshold_AutoShutdownUMS = 600000;
	// Kalman Parameters
	m_VarioParams.Kalman_VarZMeas = 400.0f;
	m_VarioParams.Kalman_VarZAccel = 1000.0f;
	m_VarioParams.Kalman_VarAccelbias = 1.0f;
	// Calibration data
	memset(&m_VarioParams.Caldata_Accel[0], 0, sizeof(m_VarioParams.Caldata_Accel));
	memset(&m_VarioParams.Caldata_Gyro[0], 0, sizeof(m_VarioParams.Caldata_Gyro));
	memset(&m_VarioParams.Caldata_Mag[0], 0, sizeof(m_VarioParams.Caldata_Mag));
	// Tone table
	memcpy(&m_VarioParams.ToneTable[0], &_ToneTable[0], sizeof(_ToneTable));
}

void CVarioPreferenceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GLIDER_TYPE, m_wndGliderType);
	DDX_CBIndex(pDX, IDC_GLIDER_TYPE, m_nGliderType);
	DDX_Text(pDX, IDC_GLIDER_MANUFACTURE, m_strGliderManufacture);
	DDX_Text(pDX, IDC_GLIDER_MODEL, m_strGliderModel);
	DDX_Text(pDX, IDC_LOGGER_TAKEOFF_SPEED, m_nLoggerTakeoffSpeed);
	DDX_Text(pDX, IDC_LOGGER_LANDING_TIMEOUT, m_nLOggerLandingTimeout);
	DDX_Text(pDX, IDC_LOGGER_LOGGING_INTERVAL, m_nLoggerLoggingInterval);
	DDX_Text(pDX, IDC_LOGGER_PILOT_NAME, m_strPilotName);
	DDX_Control(pDX, IDC_LOGGER_TIMEZONE, m_wndTimezone);
	DDX_CBIndex(pDX, IDC_LOGGER_TIMEZONE, m_nTimezone);
	DDX_Check(pDX, IDC_LOGGER_ENABLE, m_bLoggerEnable);
	DDX_Text(pDX, IDC_VARIO_CLIMB_THRESHOLD2, m_fVarioClimbThreshold);
	DDX_Text(pDX, IDC_VARIO_SINK_THRESHOLD, m_fVarioSinkThreshold);
	DDX_Text(pDX, IDC_VARIO_SENSITIVITY, m_fVarioSensitivity);
	DDX_Control(pDX, IDC_VARIO_SENTENCE, m_wndVarioSentence);
	DDX_CBIndex(pDX, IDC_VARIO_SENTENCE, m_nVarioSentence);
	DDX_Check(pDX, IDC_VARIO_BARO_ONLY, m_bVarioUseBaroOnly);
	DDX_Text(pDX, IDC_KALMAN_VAR_ZMEAS, m_fKalmanVarZmeas);
	DDX_Text(pDX, IDC_KALMAN_VAR_ZACCEL, m_fKalmanVarZaccel);
	DDX_Text(pDX, IDC_KALMAN_VAR_ACCELBIAS, m_fKalmanVarAccelbias);
	DDX_Control(pDX, IDC_VOLUME_VARIO, m_wndVolumeVario);
	DDX_Slider(pDX, IDC_VOLUME_VARIO, m_nVolumeVario);
	DDX_Text(pDX, IDC_STATIC_VOLUME_VARIO, m_strVolumeVario);
	DDX_Control(pDX, IDC_VOLUME_EFFECT, m_wndVolumeEffect);
	DDX_Slider(pDX, IDC_VOLUME_EFFECT, m_nVolumeEffect);
	DDX_Text(pDX, IDC_STATIC_VOLUME_EFFECT, m_strVolumeEffect);
	DDX_Text(pDX, IDC_THRESHOLD_LOW_BATTERY, m_fThresholdLowBattery);
	DDX_Text(pDX, IDC_THRESHOLD_SHUTDOWN_HOLDTIME, m_fThresholdShutdownHoldtime);
	DDX_Text(pDX, IDC_THRESHOLD_AUTO_POWEROFF_VARIO, m_fThresholdAutoPoweroffVario);
	DDX_Text(pDX, IDC_THRESHOLD_AUTO_POWEROFF_UMS, m_fThresholdAutoPoweroffUMS);
	DDX_Text(pDX, IDC_CALDATA_ACCEL_X, m_fCalDataAccelX);
	DDX_Text(pDX, IDC_CALDATA_ACCEL_Y, m_fCalDataAccelY);
	DDX_Text(pDX, IDC_CALDATA_ACCEL_Z, m_fCalDataAccelZ);
	DDX_Text(pDX, IDC_CALDATA_GYRO_X, m_fCalDataGyroX);
	DDX_Text(pDX, IDC_CALDATA_GYRO_Y, m_fCalDataGyroY);
	DDX_Text(pDX, IDC_CALDATA_GYRO_Z, m_fCalDataGyroZ);
}

BEGIN_MESSAGE_MAP(CVarioPreferenceDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_EDIT_TONE_TABLE, &CVarioPreferenceDlg::OnEditToneTable)
	ON_BN_CLICKED(IDC_CALIBRATION, &CVarioPreferenceDlg::OnCalibration)
	ON_BN_CLICKED(IDC_SENSOR_VIEWER, &CVarioPreferenceDlg::OnViewSensorData)
	ON_BN_CLICKED(IDC_STORE, &CVarioPreferenceDlg::OnStore)
	ON_BN_CLICKED(IDC_RELOAD, &CVarioPreferenceDlg::OnReload)
	ON_BN_CLICKED(IDC_FACTORY_RESET, &CVarioPreferenceDlg::OnFactoryReset)
	ON_COMMAND(IDC_SEND_MESSAGE, &CVarioPreferenceDlg::OnSendMessage)
END_MESSAGE_MAP()


// CVarioPreferenceDlg �޽��� ó����

BOOL CVarioPreferenceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	//
	for (int i = 0; i < sizeof(_GliderTable) / sizeof(_GliderTable[0]); i++)
		m_wndGliderType.AddString(_GliderTable[i].name);

	//
	for (int i = 0; i < sizeof(_TimezoneTable) / sizeof(_TimezoneTable[0]); i++)
		m_wndTimezone.AddString(_TimezoneTable[i].disp);

	//
	m_wndVarioSentence.AddString(_T("LK8"));
	m_wndVarioSentence.AddString(_T("LxNav"));
	//
	m_wndVolumeVario.SetRange(0, 2);
	m_wndVolumeEffect.SetRange(0, 2);

	//
	UpdateData(FALSE);

	//PostMessage(WM_COMMAND, IDC_SELECT_PORT);
	PostMessage(WM_COMMAND, MAKEWPARAM(IDC_CONNECT, BN_CLICKED), (LPARAM)GetDlgItem(IDC_CONNECT)->GetSafeHwnd());


	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CVarioPreferenceDlg::OnOK()
{
}

void CVarioPreferenceDlg::OnCancel()
{
	CDialog::OnCancel();
}



// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CVarioPreferenceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CVarioPreferenceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVarioPreferenceDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateData(TRUE);

	//
	if (pScrollBar && pScrollBar->GetSafeHwnd() == m_wndVolumeVario.GetSafeHwnd())
	{
		m_strVolumeVario = _VolumeTable[m_nVolumeVario].disp;
		UpdateData(FALSE);
	}
	if (pScrollBar && pScrollBar->GetSafeHwnd() == m_wndVolumeEffect.GetSafeHwnd())
	{
		m_strVolumeEffect = _VolumeTable[m_nVolumeEffect].disp;
		UpdateData(FALSE);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CVarioPreferenceDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;


	}

	CWnd::OnTimer(nIDEvent);
}


void CVarioPreferenceDlg::OnEditToneTable()
{
}

void CVarioPreferenceDlg::OnCalibration()
{
}

void CVarioPreferenceDlg::OnViewSensorData()
{
}

void CVarioPreferenceDlg::OnStore()
{
	//
	UpdateData(TRUE);

	//
	CString str;

	// GliderInfo
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_GLIDER_TYPE, m_VarioParams.Glider_Type);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%s\r\n"), PARAM_GLIDER_MANUFACTURE, m_VarioParams.Glider_Manufacture);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%s\r\n"), PARAM_GLIDER_MODEL, m_VarioParams.Glider_Model);
	m_SendMsgs.push_back(str);
	// IGC-Logger
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_LOGGER_ENABLE, m_VarioParams.Logger_Enable);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_LOGGER_TAKEOFF_SPEED, m_VarioParams.Logger_TakeoffSpeed);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_LOGGER_LANDING_TIMEOUT, m_VarioParams.Logger_LandingTimeout);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_LOGGER_LOGGING_INTERVAL, m_VarioParams.Logger_LoggingInterval);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%s\r\n"), PARAM_LOGGER_PILOT, m_VarioParams.Logger_PilotName);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_LOGGER_TIMEZONE, m_VarioParams.Logger_Timezone);
	m_SendMsgs.push_back(str);
	// Vario Settings
	str.Format(_T("#UP,%d,%.4f\r\n"), PARAM_VARIO_CLIMB_THRESHOLD, m_VarioParams.Vario_ClimbThreshold);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%.4f\r\n"), PARAM_VARIO_SINK_THRESHOLD, m_VarioParams.Vario_SinkThreshold);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%.4f\r\n"), PARAM_VARIO_SENSITIVITY, m_VarioParams.Vario_Sensitivity);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_VARIO_SENTENCE, m_VarioParams.Vario_Sentece);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_VARIO_BAROONLY, m_VarioParams.Vario_BaroOnly);
	m_SendMsgs.push_back(str);
	// Volume Settings
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_VOLUME_VARIO, m_VarioParams.Volume_Vario);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_VOLUME_EFFECT, m_VarioParams.Volume_Effect);
	m_SendMsgs.push_back(str);
	// Threshold Settings
	str.Format(_T("#UP,%d,%.4f\r\n"), PARAM_THRESHOLD_LOW_BATTERY, m_VarioParams.Threshold_LowBattery);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_THRESHOLD_SHUTDOWN_HOLDTIME, m_VarioParams.Threshold_ShutdownHoldtime);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_THRESHOLD_AUTO_SHUTDOWN_VARIO, m_VarioParams.Threshold_AutoShutdownVario);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%d\r\n"), PARAM_THRESHOLD_AUTO_SHUTDOWN_UMS, m_VarioParams.Threshold_AutoShutdownUMS);
	m_SendMsgs.push_back(str);
	// Kalman Parameters
	str.Format(_T("#UP,%d,%.4f\r\n"), PARAM_KALMAN_VAR_ZMEAS, m_VarioParams.Kalman_VarZMeas);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%.4f\r\n"), PARAM_KALMAN_VAR_ZACCEL, m_VarioParams.Kalman_VarZAccel);
	m_SendMsgs.push_back(str);
	str.Format(_T("#UP,%d,%.4f\r\n"), PARAM_KALMAN_VAR_ACCELBIAS, m_VarioParams.Kalman_VarAccelbias);
	m_SendMsgs.push_back(str);

	//
	m_SendMsgs.push_back(_T("#SP\r\n"));

	PostMessage(WM_COMMAND, IDC_SEND_MESSAGE);
}

void CVarioPreferenceDlg::OnReload()
{
#if 0
	//
	if (!m_bConnected)
		return;

	//
	m_SendMsgs.push_back(_T("#RP\r\n"));
	m_SendMsgs.push_back(_T("#DP\r\n"));

	//
	PostMessage(WM_COMMAND, IDC_SEND_MESSAGE);
#endif
}

void CVarioPreferenceDlg::OnFactoryReset()
{
#if 0
	//
	if (!m_bConnected)
		return;

	//
	m_SendMsgs.push_back(_T("#FR\r\n"));
	m_SendMsgs.push_back(_T("#DP\r\n"));

	//
	PostMessage(WM_COMMAND, IDC_SEND_MESSAGE);
#endif
}

void CVarioPreferenceDlg::OnSendMessage()
{
	if (m_SendMsgs.size())
	{
		CString str = m_SendMsgs.front();
		m_SendMsgs.pop_front();

		USES_CONVERSION;
		//m_Serial.Write(T2A(str));

		//
		PostMessage(WM_COMMAND, IDC_SEND_MESSAGE);
	}
}

void CVarioPreferenceDlg::ProcessReceivedMessage(WORD code, UINT param, LPCTSTR lpszData)
{
	if (code == RCODE_DUMP_PARAM || code == RCODE_QUERY_PARAM)
	{
		switch (param)
		{
		// GliderInfo
		case PARAM_GLIDER_TYPE						: // 0x1001
			m_VarioParams.Glider_Type = _ttoi(lpszData);
			break;
		case PARAM_GLIDER_MANUFACTURE				: // 0x1002
			m_VarioParams.Glider_Manufacture = lpszData;
			break;
		case PARAM_GLIDER_MODEL						: // 0x1003
			m_VarioParams.Glider_Model = lpszData;
			break;
		// IGCLogger
		case PARAM_LOGGER_ENABLE					: // 0x1101
			m_VarioParams.Logger_Enable = _ttoi(lpszData);
			break;
		case PARAM_LOGGER_TAKEOFF_SPEED				: // 0x1102
			m_VarioParams.Logger_TakeoffSpeed = _ttoi(lpszData);
			break;
		case PARAM_LOGGER_LANDING_TIMEOUT			: // 0x1103
			m_VarioParams.Logger_LandingTimeout = _ttoi(lpszData);
			break;
		case PARAM_LOGGER_LOGGING_INTERVAL			: // 0x1104
			m_VarioParams.Logger_LoggingInterval = _ttoi(lpszData);
			break;
		case PARAM_LOGGER_PILOT						: // 0x1105
			m_VarioParams.Logger_PilotName = lpszData;
			break;
		case PARAM_LOGGER_TIMEZONE					: // 0x1106
			m_VarioParams.Logger_Timezone = _ttoi(lpszData);
			break;
		// VarioSettings
		case PARAM_VARIO_SINK_THRESHOLD				: // 0x1201
			m_VarioParams.Vario_SinkThreshold = (float)_ttof(lpszData);
			break;
		case PARAM_VARIO_CLIMB_THRESHOLD			: // 0x1202
			m_VarioParams.Vario_ClimbThreshold = (float)_ttof(lpszData);
			break;
		case PARAM_VARIO_SENSITIVITY				: // 0x1203
			m_VarioParams.Vario_Sensitivity = (float)_ttof(lpszData);
			break;
		case PARAM_VARIO_SENTENCE					: // 0x1204
			m_VarioParams.Vario_Sentece = _ttoi(lpszData);
			break;
		case PARAM_VARIO_BAROONLY					: // 0x1205
			m_VarioParams.Vario_BaroOnly = _ttoi(lpszData);
			break;
			// ToneTables 
		case PARAM_TONETABLE_00_VELOCITY			: // 0x1301
			m_VarioParams.ToneTable[0].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_00_FREQ				: // 0x1302
			m_VarioParams.ToneTable[0].freq = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_00_PERIOD				: // 0x1303
			m_VarioParams.ToneTable[0].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_00_DUTY				: // 0x1304
			m_VarioParams.ToneTable[0].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_01_VELOCITY			: // 0x1311
			m_VarioParams.ToneTable[1].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_01_FREQ				: // 0x1312
			m_VarioParams.ToneTable[1].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_01_PERIOD				: // 0x1313
			m_VarioParams.ToneTable[1].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_01_DUTY				: // 0x1314
			m_VarioParams.ToneTable[1].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_02_VELOCITY			: // 0x1321
			m_VarioParams.ToneTable[2].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_02_FREQ				: // 0x1322
			m_VarioParams.ToneTable[2].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_02_PERIOD				: // 0x1323
			m_VarioParams.ToneTable[2].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_02_DUTY				: // 0x1324
			m_VarioParams.ToneTable[2].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_03_VELOCITY			: // 0x1331
			m_VarioParams.ToneTable[3].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_03_FREQ				: // 0x1332
			m_VarioParams.ToneTable[3].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_03_PERIOD				: // 0x1333
			m_VarioParams.ToneTable[3].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_03_DUTY				: // 0x1334
			m_VarioParams.ToneTable[3].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_04_VELOCITY			: // 0x1341
			m_VarioParams.ToneTable[4].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_04_FREQ				: // 0x1342
			m_VarioParams.ToneTable[4].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_04_PERIOD				: // 0x1343
			m_VarioParams.ToneTable[4].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_04_DUTY				: // 0x1344
			m_VarioParams.ToneTable[4].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_05_VELOCITY			: // 0x1351
			m_VarioParams.ToneTable[5].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_05_FREQ				: // 0x1352
			m_VarioParams.ToneTable[5].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_05_PERIOD				: // 0x1353
			m_VarioParams.ToneTable[5].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_05_DUTY				: // 0x1354
			m_VarioParams.ToneTable[5].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_06_VELOCITY			: // 0x1361
			m_VarioParams.ToneTable[6].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_06_FREQ				: // 0x1362
			m_VarioParams.ToneTable[6].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_06_PERIOD				: // 0x1363
			m_VarioParams.ToneTable[6].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_06_DUTY				: // 0x1364
			m_VarioParams.ToneTable[6].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_07_VELOCITY			: // 0x1371
			m_VarioParams.ToneTable[7].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_07_FREQ				: // 0x1372
			m_VarioParams.ToneTable[7].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_07_PERIOD				: // 0x1373
			m_VarioParams.ToneTable[7].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_07_DUTY				: // 0x1374
			m_VarioParams.ToneTable[7].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_08_VELOCITY			: // 0x1381
			m_VarioParams.ToneTable[8].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_08_FREQ				: // 0x1382
			m_VarioParams.ToneTable[8].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_08_PERIOD				: // 0x1383
			m_VarioParams.ToneTable[8].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_08_DUTY				: // 0x1384
			m_VarioParams.ToneTable[8].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_09_VELOCITY			: // 0x1391
			m_VarioParams.ToneTable[9].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_09_FREQ				: // 0x1392
			m_VarioParams.ToneTable[9].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_09_PERIOD				: // 0x1393
			m_VarioParams.ToneTable[9].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_09_DUTY				: // 0x1394
			m_VarioParams.ToneTable[9].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_10_VELOCITY			: // 0x13A1
			m_VarioParams.ToneTable[10].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_10_FREQ				: // 0x13A2
			m_VarioParams.ToneTable[10].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_10_PERIOD				: // 0x13A3
			m_VarioParams.ToneTable[10].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_10_DUTY				: // 0x13A4
			m_VarioParams.ToneTable[10].duty = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_11_VELOCITY			: // 0x13B1
			m_VarioParams.ToneTable[11].velocity = (float)_ttof(lpszData);
			break;
		case PARAM_TONETABLE_11_FREQ				: // 0x13B2
			m_VarioParams.ToneTable[11].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_11_PERIOD				: // 0x13B3
			m_VarioParams.ToneTable[11].period = _ttoi(lpszData);
			break;
		case PARAM_TONETABLE_11_DUTY				: // 0x13B4
			m_VarioParams.ToneTable[11].duty = _ttoi(lpszData);
			break;
		// VolumeSettings
		case PARAM_VOLUME_VARIO						: // 0x1401
			m_VarioParams.Volume_Vario = _ttoi(lpszData);
			break;
		case PARAM_VOLUME_EFFECT					: // 0x1402
			m_VarioParams.Volume_Effect = _ttoi(lpszData);
			break;
		// ThresholdSettings
		case PARAM_THRESHOLD_LOW_BATTERY			: // 0x1501
			m_VarioParams.Threshold_LowBattery = (float)_ttof(lpszData);
			break;
		case PARAM_THRESHOLD_SHUTDOWN_HOLDTIME		: // 0x1502
			m_VarioParams.Threshold_ShutdownHoldtime = _ttoi(lpszData);
			break;
		case PARAM_THRESHOLD_AUTO_SHUTDOWN_VARIO	: // 0x1503
			m_VarioParams.Threshold_AutoShutdownVario = _ttoi(lpszData);
			break;
		case PARAM_THRESHOLD_AUTO_SHUTDOWN_UMS		: // 0x1504
			m_VarioParams.Threshold_AutoShutdownUMS = _ttoi(lpszData);
			break;
			// KalmanParameters
		case PARAM_KALMAN_VAR_ZMEAS					: // 0x1601
			m_VarioParams.Kalman_VarZMeas = (float)_ttof(lpszData);
			break;
		case PARAM_KALMAN_VAR_ZACCEL				: // 0x1602
			m_VarioParams.Kalman_VarZAccel = (float)_ttof(lpszData);
			break;
		case PARAM_KALMAN_VAR_ACCELBIAS				: // 0x1603
			m_VarioParams.Kalman_VarAccelbias = (float)_ttof(lpszData);
			break;
		case PARAM_KALMAN_SIGMA_P					: // 0x1611
		case PARAM_KALMAN_SIGMA_A					: // 0x1612
			break;
		// CalibrationData
		case PARAM_CALDATA_ACCEL_00					: // 0x1701
			m_VarioParams.Caldata_Accel[0] = (float)_ttof(lpszData);
			break;
		case PARAM_CALDATA_ACCEL_01					: // 0x1702
			m_VarioParams.Caldata_Accel[1] = (float)_ttof(lpszData);
			break;
		case PARAM_CALDATA_ACCEL_02					: // 0x1703
			m_VarioParams.Caldata_Accel[2] = (float)_ttof(lpszData);
			break;
		case PARAM_CALDATA_GYRO_00					: // 0x1711
			m_VarioParams.Caldata_Gyro[0] = (float)_ttof(lpszData);
			break;
		case PARAM_CALDATA_GYRO_01					: // 0x1712
			m_VarioParams.Caldata_Gyro[1] = (float)_ttof(lpszData);
			break;
		case PARAM_CALDATA_GYRO_02					: // 0x1713
			m_VarioParams.Caldata_Gyro[2] = (float)_ttof(lpszData);
			break;
		case PARAM_CALDATA_MAG_00					: // 0x1721
		case PARAM_CALDATA_MAG_01					: // 0x1722
		case PARAM_CALDATA_MAG_02					: // 0x1723
			break;

		//
		case PARAM_EOF								: // 0xFFFF
			UpdateData(FALSE);
			break;
		}

		if (code == RCODE_QUERY_PARAM)
			UpdateData(FALSE);
	}
}

BOOL CVarioPreferenceDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		// GliderInfo
		m_nGliderType = GetGliderType((GliderType)m_VarioParams.Glider_Type);
		m_strGliderManufacture = m_VarioParams.Glider_Manufacture;
		m_strGliderModel = m_VarioParams.Glider_Model;
		// IGC-Logger
		m_bLoggerEnable = m_VarioParams.Logger_Enable;
		m_nLoggerTakeoffSpeed = m_VarioParams.Logger_TakeoffSpeed;
		m_nLOggerLandingTimeout = m_VarioParams.Logger_LandingTimeout;
		m_nLoggerLoggingInterval = m_VarioParams.Logger_LoggingInterval;
		m_strPilotName = m_VarioParams.Logger_PilotName;
		m_nTimezone = GetTimeZone(m_VarioParams.Logger_Timezone);
		// Vario Settings
		m_fVarioClimbThreshold = m_VarioParams.Vario_ClimbThreshold;
		m_fVarioSinkThreshold = m_VarioParams.Vario_SinkThreshold;
		m_fVarioSensitivity = m_VarioParams.Vario_Sensitivity;
		m_nVarioSentence = GetVarioSentence((VarioSentence)m_VarioParams.Vario_Sentece);
		m_bVarioUseBaroOnly = m_VarioParams.Vario_BaroOnly;
		// Volume Settings
		m_nVolumeVario = GetVolume(m_VarioParams.Volume_Vario);
		m_nVolumeEffect = GetVolume(m_VarioParams.Volume_Effect);
		// Threshold Settings
		m_fThresholdLowBattery = m_VarioParams.Threshold_LowBattery;
		m_fThresholdShutdownHoldtime = m_VarioParams.Threshold_ShutdownHoldtime;
		m_fThresholdAutoPoweroffVario = m_VarioParams.Threshold_AutoShutdownVario;
		m_fThresholdAutoPoweroffUMS = m_VarioParams.Threshold_AutoShutdownUMS;
		// Kalman Parameters
		m_fKalmanVarZmeas = m_VarioParams.Kalman_VarZMeas;
		m_fKalmanVarZaccel = m_VarioParams.Kalman_VarZAccel;
		m_fKalmanVarAccelbias = m_VarioParams.Kalman_VarAccelbias;
		// Calibration data
		m_fCalDataAccelX = m_VarioParams.Caldata_Accel[0];
		m_fCalDataAccelY = m_VarioParams.Caldata_Accel[1];
		m_fCalDataAccelZ = m_VarioParams.Caldata_Accel[2];
		m_fCalDataGyroX = m_VarioParams.Caldata_Gyro[0];
		m_fCalDataGyroY = m_VarioParams.Caldata_Gyro[1];
		m_fCalDataGyroZ = m_VarioParams.Caldata_Gyro[2];

		//
		m_strVolumeVario = _VolumeTable[m_nVolumeVario].disp;
		m_strVolumeEffect = _VolumeTable[m_nVolumeEffect].disp;
	}

	BOOL bRet = CDialog::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		// GliderInfo
		m_VarioParams.Glider_Type = _GliderTable[m_nGliderType].type;
		m_VarioParams.Glider_Manufacture = m_strGliderManufacture;
		m_VarioParams.Glider_Model = m_strGliderModel;
		// IGC-Logger
		m_VarioParams.Logger_Enable = m_bLoggerEnable;
		m_VarioParams.Logger_TakeoffSpeed = m_nLoggerTakeoffSpeed;
		m_VarioParams.Logger_LandingTimeout = m_nLOggerLandingTimeout;
		m_VarioParams.Logger_LoggingInterval = m_nLoggerLoggingInterval;
		m_VarioParams.Logger_PilotName = m_strPilotName;
		m_VarioParams.Logger_Timezone = _TimezoneTable[m_nTimezone].offset;
		// Vario Settings
		m_VarioParams.Vario_ClimbThreshold = m_fVarioClimbThreshold;
		m_VarioParams.Vario_SinkThreshold = m_fVarioSinkThreshold;
		m_VarioParams.Vario_Sensitivity = m_fVarioSensitivity;
		m_VarioParams.Vario_Sentece = _SentenceTable[m_nVarioSentence].type;
		m_VarioParams.Vario_BaroOnly = m_bVarioUseBaroOnly;
		// Volume Settings
		m_VarioParams.Volume_Vario = _VolumeTable[m_nVolumeVario].max;
		m_VarioParams.Volume_Effect = _VolumeTable[m_nVolumeEffect].max;
		// Threshold Settings
		m_VarioParams.Threshold_LowBattery = m_fThresholdLowBattery;
		m_VarioParams.Threshold_ShutdownHoldtime = m_fThresholdShutdownHoldtime;
		m_VarioParams.Threshold_AutoShutdownVario = m_fThresholdAutoPoweroffVario;
		m_VarioParams.Threshold_AutoShutdownUMS = m_fThresholdAutoPoweroffUMS;
		// Kalman Parameters
		m_VarioParams.Kalman_VarZMeas = m_fKalmanVarZmeas;
		m_VarioParams.Kalman_VarZAccel = m_fKalmanVarZaccel;
		m_VarioParams.Kalman_VarAccelbias = m_fKalmanVarAccelbias;
	}

	return bRet;
}
