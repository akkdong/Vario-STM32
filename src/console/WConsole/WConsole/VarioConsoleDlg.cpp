
// VarioConsoleDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "wconsole.h"
#include "VarioConsoleDlg.h"
#include "afxdialogex.h"
#include "SerialPortSelectDlg.h"
#include "VarioPreferenceDlg.h"

#include <strsafe.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



static void ShowLastError(LPTSTR lpszFunction, DWORD dwLastError)
{
	// Retrieve the system error message for the last-error code
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
					LocalSize(lpDisplayBuf) / sizeof(TCHAR),
					TEXT("%s failed with error %d: %s"),
					lpszFunction, dwLastError, lpMsgBuf);

	AfxMessageBox((LPCTSTR)lpDisplayBuf, MB_ICONERROR | MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
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

// CVarioConsoleDlg 대화 상자

CVarioConsoleDlg::CVarioConsoleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_WCONSOLE_DIALOG, pParent)
	, m_bConnected(FALSE)
	, m_nBufLen(0)
	, m_bRecvVarioMsg(FALSE)
	, m_nTimerID(0)
	, m_nPortNum(0)
	, m_nBaudRate(CSerial::EBaud115200)
	, m_nDataBits(CSerial::EData8)
	, m_nParity(CSerial::EParNone)
	, m_nStopBits(CSerial::EStop1)
	, m_strLatitude(_T("37.452329 E"))
	, m_strLongitude(_T("126.360923 N"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CVarioConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LATITUDE, m_wndLatitude);
	DDX_Control(pDX, IDC_LONGITUDE, m_wndLongitude);
	DDX_Text(pDX, IDC_LATITUDE, m_strLatitude);
	DDX_Text(pDX, IDC_LONGITUDE, m_strLongitude);
}

BEGIN_MESSAGE_MAP(CVarioConsoleDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(CSerialWnd::mg_nDefaultComMsg, OnSerialMessage)
	ON_BN_CLICKED(IDC_EDIT_TONE_TABLE, &CVarioConsoleDlg::OnEditToneTable)
	ON_BN_CLICKED(IDC_CALIBRATION, &CVarioConsoleDlg::OnCalibration)
	ON_BN_CLICKED(IDC_SENSOR_VIEWER, &CVarioConsoleDlg::OnViewSensorData)
	ON_BN_CLICKED(IDC_CONNECT, &CVarioConsoleDlg::OnConnect)
	ON_BN_CLICKED(IDC_RELOAD, &CVarioConsoleDlg::OnReload)
	ON_BN_CLICKED(IDC_PREFERENCES, &CVarioConsoleDlg::OnPreferences)
	ON_COMMAND(IDC_SELECT_PORT, &CVarioConsoleDlg::OnSelectPort)
	ON_COMMAND(IDC_SEND_MESSAGE, &CVarioConsoleDlg::OnSendMessage)
END_MESSAGE_MAP()


// CVarioConsoleDlg 메시지 처리기

BOOL CVarioConsoleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	//
	m_wndLatitude.SetNumberOfLines(1);
	m_wndLatitude.SetXCharsPerLine(11); // 37.234234 E
	m_wndLatitude.SetSize(CMatrixStatic::SMALL);
	m_wndLatitude.SetDisplayColors(RGB(0, 0, 0), RGB(255, 60, 0), RGB(103, 30, 0));
	m_wndLatitude.AdjustClientXToSize(11);
	m_wndLatitude.AdjustClientYToSize(1);

	m_wndLongitude.SetNumberOfLines(1);
	m_wndLongitude.SetXCharsPerLine(12); // 37.234234 E
	m_wndLongitude.SetSize(CMatrixStatic::SMALL);
	m_wndLongitude.SetDisplayColors(RGB(0, 0, 0), RGB(255, 60, 0), RGB(103, 30, 0));
	m_wndLongitude.AdjustClientXToSize(12);
	m_wndLongitude.AdjustClientYToSize(1);

	//
	UpdateData(FALSE);
	UpdateTitle();

	//PostMessage(WM_COMMAND, IDC_SELECT_PORT);
	PostMessage(WM_COMMAND, MAKEWPARAM(IDC_CONNECT, BN_CLICKED), (LPARAM)GetDlgItem(IDC_CONNECT)->GetSafeHwnd());


	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CVarioConsoleDlg::OnOK()
{
}

void CVarioConsoleDlg::OnCancel()
{
	m_Serial.Close();

	CDialog::OnCancel();
}



// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CVarioConsoleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CVarioConsoleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVarioConsoleDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;

		if (! m_bRecvVarioMsg)
		{
			CloseSerial();
			UpdateTitle();

			AfxMessageBox(_T("Device is not reponsding!"), MB_OK | MB_ICONEXCLAMATION);
		}
	}

	CWnd::OnTimer(nIDEvent);
}

LRESULT CVarioConsoleDlg::OnSerialMessage(WPARAM wParam, LPARAM lParam)
{
	CSerial::EEvent eEvent = CSerial::EEvent(LOWORD(wParam));
	CSerial::EError eError = CSerial::EError(HIWORD(wParam));

#if 0
	if (eError)
		AfxMessageBox(_T("An internal error occurred."), MB_OK);

	if (eEvent & CSerial::EEventBreak)
		AfxMessageBox(_T("Break detected on input."), MB_OK);

	if (eEvent & CSerial::EEventError)
		AfxMessageBox(_T("A line-status error occurred."), MB_OK);

	if (eEvent & CSerial::EEventRcvEv)
		AfxMessageBox(_T("Event character has been received."), MB_OK);

	if (eEvent & CSerial::EEventRing)
		AfxMessageBox(_T("Ring detected"), MB_OK);

	if (eEvent & CSerial::EEventSend)
		AfxMessageBox(_T("All data is send"), MB_OK);

	if (eEvent & CSerial::EEventCTS)
		AfxMessageBox(_T("CTS signal change"), MB_OK);

	if (eEvent & CSerial::EEventDSR)
		AfxMessageBox(_T("DSR signal change"), MB_OK);

	if (eEvent & CSerial::EEventRLSD)
		AfxMessageBox(_T("RLSD signal change"), MB_OK);
#endif

	if (eEvent & CSerial::EEventRecv)
	{
		CHAR ch;
		DWORD dwRead;

		while (m_Serial.Read(&ch, sizeof(ch), &dwRead) == ERROR_SUCCESS && dwRead == sizeof(ch))
		{
			if (ch != '\r' && ch != '\n')
			{
				m_pSerialBuf[m_nBufLen++] = ch;
				m_pSerialBuf[m_nBufLen] = '\0';
			}
			else
			{
				if (m_nBufLen > 0)
				{
					USES_CONVERSION;
					//TRACE("%s\n", m_pSerialBuf);
					m_RecvMsgs.push_back(CString(A2T(m_pSerialBuf)));

					m_pSerialBuf[0] = '\0';
					m_nBufLen = 0;

					ParseReceivedMessage();
				}
			}
		}
	}
	else if (eEvent & CSerial::EEventSend)
	{
		TRACE("Last character on output was sent");
	}

	return 0;
}

void CVarioConsoleDlg::OnEditToneTable()
{
	//
	//if (!m_bConnected)
	//	return;

	Beep(1200, 500);
	Sleep(500);
	Beep(1200, 500);
	Sleep(500);
	Beep(1200, 500);
}

void CVarioConsoleDlg::OnCalibration()
{
	//
	if (!m_bConnected)
		return;

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CVarioConsoleDlg::OnViewSensorData()
{
	//
	if (!m_bConnected)
		return;

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CVarioConsoleDlg::OnConnect()
{
	if (!m_bConnected)
	{
		CSerialPortSelectDlg dlg;

		dlg.m_sPortNum		= m_nPortNum;
		dlg.m_sBaudRate		= m_nBaudRate;
		dlg.m_sDataBits		= m_nDataBits;
		dlg.m_sParity		= m_nParity;
		dlg.m_sStopBits		= m_nStopBits;

		if (dlg.DoModal() == IDOK)
		{
			//
			m_nPortNum = dlg.m_sPortNum;
			m_nBaudRate = dlg.m_sBaudRate;
			m_nDataBits = dlg.m_sDataBits;
			m_nParity = dlg.m_sParity;
			m_nStopBits = dlg.m_sStopBits;

			//
			OpenSerial(dlg.m_sPortName, dlg.m_sBaudRate, dlg.m_sDataBits, dlg.m_sParity, dlg.m_sStopBits, dlg.m_sFlowControl);
		}
	}
	else
	{
		//
		CloseSerial();

	}

	UpdateTitle();
}

void CVarioConsoleDlg::OnReload()
{
	//
	if (!m_bConnected)
		return;

	//
	m_SendMsgs.push_back(_T("#RP\r\n"));
	m_SendMsgs.push_back(_T("#DP\r\n"));

	//
	PostMessage(WM_COMMAND, IDC_SEND_MESSAGE);
}

void CVarioConsoleDlg::CVarioConsoleDlg::OnPreferences()
{
	if (m_bConnected && m_bRecvVarioMsg)
	{
		CVarioPreferenceDlg dlg;

		dlg.DoModal();
	}
}

void CVarioConsoleDlg::OnSelectPort()
{

}

void CVarioConsoleDlg::OnSendMessage()
{
	if (m_SendMsgs.size())
	{
		CString str = m_SendMsgs.front();
		m_SendMsgs.pop_front();

		USES_CONVERSION;
		m_Serial.Write(T2A(str));

		//
		PostMessage(WM_COMMAND, IDC_SEND_MESSAGE);
	}
}

void CVarioConsoleDlg::ParseReceivedMessage()
{
	while (m_RecvMsgs.begin() != m_RecvMsgs.end())
	{
		CString strLine = m_RecvMsgs.front();
		m_RecvMsgs.pop_front();

		if (strLine.GetLength() == 0)
			continue;

		if (strLine.GetAt(0) == _T('%'))
		{
			// %XX,<param>,<data1>,<data2>,...
			CString strCode, strParam, strData;
			CString strToken = _T(",");
			int nStart = 1;

			strCode = strLine.Tokenize(strToken, nStart);
			if (nStart > 0)
				strParam = strLine.Tokenize(strToken, nStart);
			if (nStart > 0)
				strData = strLine.Tokenize(strToken, nStart);

			if (strCode.GetLength() == 2)
			{
				TRACE(_T(">>> %s: %s, %s\n"), strCode, strParam, strData);
				ProcessReceivedMessage(PStrToCode(strCode), _ttoi(strParam), strData);

				m_bRecvVarioMsg = TRUE;
			}
		}
		else if (strLine.GetAt(0) == _T('$'))
		{
			// processing NMEA setence
			TRACE(_T(">>> %s\n"), strLine);

			m_bRecvVarioMsg = TRUE;
		}
	}
}

void CVarioConsoleDlg::ProcessReceivedMessage(WORD code, UINT param, LPCTSTR lpszData)
{
	if (code == CMD_DUMP_PROPERTY || code == CMD_QUERY_PARAM)
	{
		switch (param)
		{
		// GliderInfo
		case PARAM_GLIDER_TYPE						: // 0x1001
			break;
		case PARAM_GLIDER_MANUFACTURE				: // 0x1002
			break;
		case PARAM_GLIDER_MODEL						: // 0x1003
			break;
		// IGCLogger
		case PARAM_LOGGER_ENABLE					: // 0x1101
			break;
		case PARAM_LOGGER_TAKEOFF_SPEED				: // 0x1102
			break;
		case PARAM_LOGGER_LANDING_TIMEOUT			: // 0x1103
			break;
		case PARAM_LOGGER_LOGGING_INTERVAL			: // 0x1104
			break;
		case PARAM_LOGGER_PILOT						: // 0x1105
			break;
		case PARAM_LOGGER_TIMEZONE					: // 0x1106
			break;
		// VarioSettings
		case PARAM_VARIO_SINK_THRESHOLD				: // 0x1201
			break;
		case PARAM_VARIO_CLIMB_THRESHOLD			: // 0x1202
			break;
		case PARAM_VARIO_SENSITIVITY				: // 0x1203
			break;
		case PARAM_VARIO_SENTENCE					: // 0x1204
			break;
		case PARAM_VARIO_BAROONLY					: // 0x1205
			break;
			// ToneTables 
		case PARAM_TONETABLE_00_VELOCITY			: // 0x1301
			break;
		case PARAM_TONETABLE_00_FREQ				: // 0x1302
			break;
		case PARAM_TONETABLE_00_PERIOD				: // 0x1303
			break;
		case PARAM_TONETABLE_00_DUTY				: // 0x1304
			break;
		case PARAM_TONETABLE_01_VELOCITY			: // 0x1311
			break;
		case PARAM_TONETABLE_01_FREQ				: // 0x1312
			break;
		case PARAM_TONETABLE_01_PERIOD				: // 0x1313
			break;
		case PARAM_TONETABLE_01_DUTY				: // 0x1314
			break;
		case PARAM_TONETABLE_02_VELOCITY			: // 0x1321
			break;
		case PARAM_TONETABLE_02_FREQ				: // 0x1322
			break;
		case PARAM_TONETABLE_02_PERIOD				: // 0x1323
			break;
		case PARAM_TONETABLE_02_DUTY				: // 0x1324
			break;
		case PARAM_TONETABLE_03_VELOCITY			: // 0x1331
			break;
		case PARAM_TONETABLE_03_FREQ				: // 0x1332
			break;
		case PARAM_TONETABLE_03_PERIOD				: // 0x1333
			break;
		case PARAM_TONETABLE_03_DUTY				: // 0x1334
			break;
		case PARAM_TONETABLE_04_VELOCITY			: // 0x1341
			break;
		case PARAM_TONETABLE_04_FREQ				: // 0x1342
			break;
		case PARAM_TONETABLE_04_PERIOD				: // 0x1343
			break;
		case PARAM_TONETABLE_04_DUTY				: // 0x1344
			break;
		case PARAM_TONETABLE_05_VELOCITY			: // 0x1351
			break;
		case PARAM_TONETABLE_05_FREQ				: // 0x1352
			break;
		case PARAM_TONETABLE_05_PERIOD				: // 0x1353
			break;
		case PARAM_TONETABLE_05_DUTY				: // 0x1354
			break;
		case PARAM_TONETABLE_06_VELOCITY			: // 0x1361
			break;
		case PARAM_TONETABLE_06_FREQ				: // 0x1362
			break;
		case PARAM_TONETABLE_06_PERIOD				: // 0x1363
			break;
		case PARAM_TONETABLE_06_DUTY				: // 0x1364
			break;
		case PARAM_TONETABLE_07_VELOCITY			: // 0x1371
			break;
		case PARAM_TONETABLE_07_FREQ				: // 0x1372
			break;
		case PARAM_TONETABLE_07_PERIOD				: // 0x1373
			break;
		case PARAM_TONETABLE_07_DUTY				: // 0x1374
			break;
		case PARAM_TONETABLE_08_VELOCITY			: // 0x1381
			break;
		case PARAM_TONETABLE_08_FREQ				: // 0x1382
			break;
		case PARAM_TONETABLE_08_PERIOD				: // 0x1383
			break;
		case PARAM_TONETABLE_08_DUTY				: // 0x1384
			break;
		case PARAM_TONETABLE_09_VELOCITY			: // 0x1391
			break;
		case PARAM_TONETABLE_09_FREQ				: // 0x1392
			break;
		case PARAM_TONETABLE_09_PERIOD				: // 0x1393
			break;
		case PARAM_TONETABLE_09_DUTY				: // 0x1394
			break;
		case PARAM_TONETABLE_10_VELOCITY			: // 0x13A1
			break;
		case PARAM_TONETABLE_10_FREQ				: // 0x13A2
			break;
		case PARAM_TONETABLE_10_PERIOD				: // 0x13A3
			break;
		case PARAM_TONETABLE_10_DUTY				: // 0x13A4
			break;
		case PARAM_TONETABLE_11_VELOCITY			: // 0x13B1
			break;
		case PARAM_TONETABLE_11_FREQ				: // 0x13B2
			break;
		case PARAM_TONETABLE_11_PERIOD				: // 0x13B3
			break;
		case PARAM_TONETABLE_11_DUTY				: // 0x13B4
			break;
		// VolumeSettings
		case PARAM_VOLUME_VARIO						: // 0x1401
			break;
		case PARAM_VOLUME_EFFECT					: // 0x1402
			break;
		case PARAM_VOLUME_TURNON_AT_TAKEOFF			: // 0x1403
			break;
		// ThresholdSettings
		case PARAM_THRESHOLD_LOW_BATTERY			: // 0x1501
			break;
		case PARAM_THRESHOLD_SHUTDOWN_HOLDTIME		: // 0x1502
			break;
		case PARAM_THRESHOLD_AUTO_SHUTDOWN_VARIO	: // 0x1503
			break;
		case PARAM_THRESHOLD_AUTO_SHUTDOWN_UMS		: // 0x1504
			break;
			// KalmanParameters
		case PARAM_KALMAN_VAR_ZMEAS					: // 0x1601
			break;
		case PARAM_KALMAN_VAR_ZACCEL				: // 0x1602
			break;
		case PARAM_KALMAN_VAR_ACCELBIAS				: // 0x1603
			break;
		case PARAM_KALMAN_SIGMA_P					: // 0x1611
		case PARAM_KALMAN_SIGMA_A					: // 0x1612
			break;
		// CalibrationData
		case PARAM_CALDATA_ACCEL_00					: // 0x1701
			break;
		case PARAM_CALDATA_ACCEL_01					: // 0x1702
			break;
		case PARAM_CALDATA_ACCEL_02					: // 0x1703
			break;
		case PARAM_CALDATA_GYRO_00					: // 0x1711
			break;
		case PARAM_CALDATA_GYRO_01					: // 0x1712
			break;
		case PARAM_CALDATA_GYRO_02					: // 0x1713
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

		//if (code == RCODE_QUERY_PARAM)
		if (code == CMD_QUERY_PARAM)
			UpdateData(FALSE);
	}
}

BOOL CVarioConsoleDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		m_wndLatitude.SetText(m_strLatitude);
		m_wndLongitude.SetText(m_strLongitude);
	}

	BOOL bRet = CDialog::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		
	}

	return bRet;
}

void CVarioConsoleDlg::UpdateTitle()
{
	CString strTitle;

	if (m_bConnected)
	{
		int b = 0, d = 0;
		TCHAR p = _T(' ');
		TCHAR * s = _T("");

		switch (m_nBaudRate)
		{
		case CSerial::EBaud9600		: b = 9600;		break;
		case CSerial::EBaud14400	: b = 14400;	break;
		case CSerial::EBaud19200	: b = 19200;	break;
		case CSerial::EBaud38400	: b = 38400;	break;
		case CSerial::EBaud56000	: b = 56000;	break;
		case CSerial::EBaud57600	: b = 57600;	break;
		case CSerial::EBaud115200	: b = 115200;	break;
		}

		switch (m_nDataBits)
		{
		case CSerial::EData5	: d = 5; break;
		case CSerial::EData6	: d = 6; break;
		case CSerial::EData7	: d = 7; break;
		case CSerial::EData8	: d = 8; break;
		}

		switch (m_nParity)
		{
		case CSerial::EParNone	: p = _T('N'); break;
		case CSerial::EParOdd	: p = _T('O'); break;
		case CSerial::EParEven	: p = _T('E'); break;
		case CSerial::EParMark	: p = _T('M'); break;
		case CSerial::EParSpace	: p = _T('S'); break;
		}

		switch (m_nStopBits)
		{
		case CSerial::EStop1	: s = _T("1");	break;
		case CSerial::EStop1_5	: s = _T("1.5");break;
		case CSerial::EStop2	: s = _T("2");	break;
			
		}

		strTitle.Format(_T("Variometer - COM%d:%d,%d,%c,%s"), m_nPortNum, b, d, p, s);
	}
	else
	{
		strTitle.Format(_T("Variometer - Unconnected!"));
	}

	//
	GetDlgItem(IDC_CONNECT)->SetWindowText((m_bConnected ? _T("Disconnect") : _T("Connect")));
	//
	SetWindowText(strTitle);
}

void CVarioConsoleDlg::OpenSerial(LPCTSTR lpszDevice, CSerial::EBaudrate nBaudRate, CSerial::EDataBits nDataBits, CSerial::EParity nParity, CSerial::EStopBits nStopBits, CSerial::EHandshake nHandshake)
{
	CWaitCursor wait;
	LONG lResult;

	if ((lResult = m_Serial.Open(lpszDevice, m_hWnd)) == ERROR_SUCCESS)
	{
		//
		m_Serial.Setup(nBaudRate, nDataBits, nParity, nStopBits);
		m_Serial.SetupHandshaking(nHandshake);
		m_Serial.SetupReadTimeouts(CSerial::EReadTimeoutNonblocking);

		m_bConnected = TRUE;
		m_bRecvVarioMsg = FALSE;


		//
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_RELOAD, BN_CLICKED), (LPARAM)GetDlgItem(IDC_RELOAD)->GetSafeHwnd());
		//
		m_nTimerID = SetTimer(TIMER_CHECK_RESPONSE, 2000, NULL);
	}
	else
	{
		ShowLastError(_T("Serial connection"), lResult);
	}
}
void CVarioConsoleDlg::CloseSerial()
{
	m_Serial.Close();

	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

	//
	m_RecvMsgs.clear();
	m_SendMsgs.clear();

	m_bConnected = FALSE;
}
