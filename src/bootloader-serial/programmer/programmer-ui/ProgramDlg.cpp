#include "stdafx.h"
#include "resource.h"
#include "ProgramDlg.h"
#include "DownloaderDlg.h"


#define TIMER_CHECK_RESPONSE		(0x5911)

#define WM_INTERNAL_ERROR			(WM_USER+5)


////////////////////////////////////////////////////////////////////////////////////
//

CProgramDlg::CProgramDlg(CDownloaderDlg * pParent)
	: CDialogEx(IDD_PROGRAM, pParent)
	, m_bVerifyProgram(TRUE)
	, m_bRunAfterProgram(FALSE)
	, m_pDlgMain(pParent)
	, m_nTimerID(0)
	, m_nState(_READY)
	, m_pFirmware(NULL)
{
}

CProgramDlg::~CProgramDlg()
{
	Cleanup();
}

void CProgramDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_FILE_PATH, m_strFilePath);
	DDX_Check(pDX, IDC_VERIFY_PROGRAM, m_bVerifyProgram);
	DDX_Check(pDX, IDC_RUN_PROGRAM, m_bRunAfterProgram);
}


BEGIN_MESSAGE_MAP(CProgramDlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_COMMAND(IDC_EXECUTE, OnExecute)
	ON_MESSAGE(WM_INTERNAL_ERROR, OnError)
END_MESSAGE_MAP()


BOOL CProgramDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}

void CProgramDlg::OnOK()
{
}

void CProgramDlg::OnCancel()
{
	if (m_nState != _READY)
	{
		UINT nID = AfxMessageBox("Do you wants to stop programming & close this dialog", MB_ICONSTOP);

		if (nID != IDOK)
			return;
	}

	CDialogEx::OnCancel();
}

BOOL CProgramDlg::UpdateData(BOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		// Local variables -> control variables
	}

	BOOL bRet = CDialogEx::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		// Control variables -> local variables
	}

	return bRet;
}

void CProgramDlg::RequestErase(uint32_t address)
{
	CommandMaker maker;

	maker.start(HCODE_ERASE);
	maker.push_u32(address);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::RequestProgram(uint32_t address)
{
	CommandMaker maker;

	maker.start(HCODE_WRITE);
	maker.push_u32(address);
	maker.push_mem(&m_pFirmware[address - m_nProgramStart], PROGRAM_SIZE);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::RequestVerify(uint32_t address)
{
	CommandMaker maker;

	maker.start(HCODE_READ);
	maker.push_u32(address);
	maker.push_u16(PROGRAM_SIZE);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::RequestRun()
{
	CommandMaker maker;

	maker.start(HCODE_START);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::OnBrowse()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, _T("Firmware files (*.bin;*.hex)|*.bin;*.hex|All files (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		UpdateData(TRUE);
		m_strFilePath = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

BOOL CProgramDlg::FileExist(LPCTSTR lpszFile, uint32_t * pSize)
{
	CFileStatus status;

	if (CFile::GetStatus(lpszFile, status))
	{
		if (pSize)
			*pSize = (uint32_t)status.m_size;

		return TRUE;
	}

	return FALSE;
}

BOOL CProgramDlg::LoadFile(LPCTSTR lpszFile)
{
	CFile file;

	if (file.Open(lpszFile, CFile::modeRead))
	{
		m_nFileSize = (uint32_t)file.SeekToEnd();
		m_nProgramSize = (m_nFileSize + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

		m_pFirmware = (uint8_t *)malloc(m_nProgramSize);

		if (m_pFirmware != NULL)
		{
			memset(m_pFirmware, 0xFF, m_nProgramSize);

			file.SeekToBegin();
			file.Read(m_pFirmware, m_nFileSize);
		}

		file.Close();
	}

	return ((m_pFirmware != NULL) ? TRUE : FALSE);
}

void CProgramDlg::EnableControls()
{
	GetDlgItem(IDC_FILE_PATH)->EnableWindow(m_nState == _READY ? TRUE : FALSE);
	GetDlgItem(IDC_BROWSE)->EnableWindow(m_nState == _READY ? TRUE : FALSE);
	GetDlgItem(IDC_VERIFY_PROGRAM)->EnableWindow(m_nState == _READY ? TRUE : FALSE);
	GetDlgItem(IDC_RUN_PROGRAM)->EnableWindow(m_nState == _READY ? TRUE : FALSE);

	GetDlgItem(IDC_START)->SetWindowTextA(m_nState == _READY ? "Start" : "Stop");
}

void CProgramDlg::Cleanup()
{
	if (m_pFirmware != NULL)
	{
		free(m_pFirmware);
		m_pFirmware = NULL;
	}
}

void CProgramDlg::OnStart()
{
	if (m_nState == _READY)
	{
		UpdateData();

		m_strFilePath.TrimLeft();
		m_strFilePath.TrimRight();

		if (m_strFilePath.IsEmpty() || !FileExist(m_strFilePath))
		{
			AfxMessageBox("Enter firmware file path", MB_ICONSTOP);
			GotoDlgCtrl(GetDlgItem(IDC_FILE_PATH));
		}
		else
		{
			Cleanup();

			if (LoadFile(m_strFilePath))
			{
				//
				m_nProgramStart = ADDRESS_USER_APPLICATION;
				m_nProgramEnd = m_nProgramStart + m_nProgramSize;

				m_nTotalPage = (m_nProgramEnd - m_nProgramStart) / PAGE_SIZE;
				m_nActivePage = 0;
				m_nActiveSubPage = 0;

				m_nState = _RUN;

				//
				PostMessage(WM_COMMAND, IDC_EXECUTE);
				EnableControls();
			}
		}
	}
	else
	{
		// cancel
		m_nState = _READY;

		//
		if (m_nTimerID)
		{
			KillTimer(m_nTimerID);
			m_nTimerID = 0;
		}

		EnableControls();
	}
}

void CProgramDlg::OnExecute()
{
	if (m_nState == _RUN)
	{
		if (m_nActivePage < m_nTotalPage)
		{
			RequestErase(m_nProgramStart + m_nActivePage * PAGE_SIZE);
			SetTimer(TIMER_CHECK_RESPONSE, 2000, NULL);

			m_nState = _ERASE;
		}
		else
		{
			//
			m_nState = _DONE;

			//
			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
	}
	else if (m_nState == _ERASE)
	{
		//
		m_nActiveSubPage = 0;
		m_nState = _PROGRAM;

		//
		PostMessage(WM_COMMAND, IDC_EXECUTE);
	}
	else if (m_nState == _PROGRAM)
	{
		if (m_nActiveSubPage < PROGRAM_COUNT)
		{
			RequestProgram(m_nProgramStart + m_nActivePage * PAGE_SIZE + m_nActiveSubPage * PROGRAM_SIZE);
			SetTimer(TIMER_CHECK_RESPONSE, 2000, NULL);
		}
		else
		{
			if (m_bVerifyProgram)
			{
				m_nActiveSubPage = 0;
				m_nState = _VERIFY;
			}
			else
			{
				m_nActivePage = m_nActivePage + 1;
				m_nState = _RUN;
			}

			//
			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
	}
	else if (m_nState == _VERIFY)
	{
		if (m_nActiveSubPage < PROGRAM_COUNT)
		{
			RequestVerify(m_nProgramStart + m_nActivePage * PAGE_SIZE + m_nActiveSubPage * PROGRAM_SIZE);
			SetTimer(TIMER_CHECK_RESPONSE, 2000, NULL);
		}
		else
		{
			//
			m_nActivePage = m_nActivePage + 1;
			m_nState = _RUN;

			//
			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
	}
	else if (m_nState == _DONE)
	{
		if (m_bRunAfterProgram)
			RequestRun();
		else
			AfxMessageBox("Done!!", MB_ICONINFORMATION);

		CDialogEx::EndDialog(m_bRunAfterProgram ? IDYES : IDOK);
	}
}

void CProgramDlg::OnPacketReceived(PACKET * pPacket)
{
	// reset timer
	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

	//
	if (m_nState == _ERASE)
	{
		if (pPacket->code == DCODE_ACK)
		{
			// erase success
			// nop

			// continue
			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
		else if (pPacket->code == DCODE_NACK)
		{
			// error handling
			PostMessage(WM_USER + 5, pPacket->e.error);
		}
	}
	else if (m_nState == _PROGRAM)
	{
		if (pPacket->code == DCODE_ACK)
		{
			// program success
			m_nActiveSubPage += 1;

			// continue
			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
		else if (pPacket->code == DCODE_NACK)
		{
			// error handling
			PostMessage(WM_INTERNAL_ERROR, pPacket->e.error);
		}
	}
	else if (m_nState == _VERIFY)
	{
		if (pPacket->code == DCODE_DUMP_MEM)
		{
			// compare program data with read data
			uint32_t addr = /*m_nProgramStart +*/ m_nActivePage * PAGE_SIZE + m_nActiveSubPage * PROGRAM_SIZE;
			int i;

			for (i = 0; i < PROGRAM_SIZE; i++)
				if (m_pFirmware[addr + i] != pPacket->d.data[i])
					break; // OOPS!!

			if (i >= PROGRAM_SIZE)
			{
				// verfiy success
				m_nActiveSubPage += 1;

				// continue
				PostMessage(WM_COMMAND, IDC_EXECUTE);
			}
			else
			{
				// error handling
				PostMessage(WM_INTERNAL_ERROR, ERROR_MEMORY_COLLAPSE);
			}
		}
		else if (pPacket->code == DCODE_NACK)
		{
			// error handling
			PostMessage(WM_INTERNAL_ERROR, pPacket->e.error);
		}
	}


	//	if (pPacket->code == DCODE_DUMP_MEM)
	//		m_wndHexEdit.SetData(pPacket->payloadLen - 4, pPacket->d.data);
}

void CProgramDlg::OnTimer(UINT nEventID)
{	
	CDialogEx::OnTimer(nEventID);

	if (nEventID == m_nTimerID)
	{
		// error handling : timeout
		PostMessage(WM_INTERNAL_ERROR, ERROR_RESPONSE_TIMEOUT);

		//
		KillTimer(m_nTimerID);
		m_nTimerID;
	}
}

LRESULT CProgramDlg::OnError(WPARAM wParam, LPARAM lParam)
{
	return 0;
}