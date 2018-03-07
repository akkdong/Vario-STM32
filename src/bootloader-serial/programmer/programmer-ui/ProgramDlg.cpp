#include "stdafx.h"
#include "resource.h"
#include "ProgramDlg.h"
#include "DownloaderDlg.h"
#include "BinFile.h"
#include "HexFile.h"
#include "Flash.h"


#define TIMER_CHECK_RESPONSE		(0x5911)

#define WM_DEVICE_ERROR				(WM_USER+5)


////////////////////////////////////////////////////////////////////////////////////
//

CProgramDlg::CProgramDlg(CDownloaderDlg * pParent)
	: CDialogEx(IDD_PROGRAM, pParent)
	, m_pDlgMain(pParent)
	, m_nTimerID(0)
	, m_nState(_READY)
	, m_pFirmware(NULL)
{
	m_bVerifyProgram = AfxGetApp()->GetProfileIntA(_T("ProgramSettings"), _T("Verify"), 1);
	m_bRunAfterProgram = AfxGetApp()->GetProfileIntA(_T("ProgramSettings"), _T("Run"), 0);

	m_strFilePath = AfxGetApp()->GetProfileStringA(_T("ProgramSettings"), _T("Path"), _T(""));
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
	ON_MESSAGE(WM_DEVICE_ERROR, OnError)
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
	BPacketMaker maker;

	maker.start(HCODE_ERASE);
	maker.push_u32(address);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::RequestErase(uint32_t start, uint32_t end)
{
	BPacketMaker maker;

	maker.start(HCODE_ERASE);
	maker.push_u32(start);
	maker.push_u32(end);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::RequestProgram(uint32_t address)
{
	BPacketMaker maker;

	maker.start(HCODE_WRITE);
	maker.push_u32(address);
	maker.push_mem(&m_pFirmware[address - m_nProgramStart], PROGRAM_SIZE);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::RequestVerify(uint32_t address)
{
	BPacketMaker maker;

	maker.start(HCODE_READ);
	maker.push_u32(address);
	maker.push_u16(PROGRAM_SIZE);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::RequestRun()
{
	BPacketMaker maker;

	maker.start(HCODE_START);
	maker.finish();

	m_pDlgMain->Log(CDownloaderDlg::_INFO, "Jump to user application!");
	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}

void CProgramDlg::OnBrowse()
{
	UpdateData();

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, _T("Firmware files (*.bin;*.hex)|*.bin;*.hex|All files (*.*)|*.*||"));
	dlg.m_ofn.lpstrInitialDir = m_strFilePath;

	if (dlg.DoModal() == IDOK)
	{
		UpdateData(TRUE);
		m_strFilePath = dlg.GetPathName();
		AfxGetApp()->WriteProfileStringA(_T("ProgramSettings"), _T("Path"), m_strFilePath);
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
	CString strFile(lpszFile);
	ImageFile * pImageFile;

	if (strFile.Right(4).CompareNoCase(_T(".hex")) == 0)
		pImageFile = new HexFile;
	else
		pImageFile = new BinFile;

	if (pImageFile->Open(strFile, 0) == ImageFile::PARSER_ERR_OK)
	{
		m_nFileSize = (uint32_t)pImageFile->getSize();
		m_nProgramSize = (m_nFileSize + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

		m_pFirmware = (uint8_t *)malloc(m_nProgramSize);

		if (m_pFirmware != NULL)
		{
			memset(m_pFirmware, 0xFF, m_nProgramSize);
			pImageFile->Read(m_pFirmware, &m_nFileSize);
		}
	}

	delete pImageFile;

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

		AfxGetApp()->WriteProfileInt(_T("ProgramSettings"), _T("Verify"), m_bVerifyProgram);
		AfxGetApp()->WriteProfileInt(_T("ProgramSettings"), _T("Run"), m_bRunAfterProgram);
		AfxGetApp()->WriteProfileStringA(_T("ProgramSettings"), _T("Path"), m_strFilePath);

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
				m_nActiveBlock = 0;

				m_nState = _RUN;

				//
				PostMessage(WM_COMMAND, IDC_EXECUTE);
				EnableControls();
				m_pDlgMain->Log(CDownloaderDlg::_INFO, "Start programming: total %d page(s)", m_nTotalPage);
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
		m_pDlgMain->Log(CDownloaderDlg::_INFO, "Erase from 0x%08X to 0x%08X", m_nProgramStart, m_nProgramStart + (m_nTotalPage - 1) * PAGE_SIZE);
		RequestErase(m_nProgramStart, m_nProgramStart + (m_nTotalPage - 1) * PAGE_SIZE); // or m_nProgramStart ~ m_nProgramEnd - PAGE_SIZE
		SetTimer(TIMER_CHECK_RESPONSE, 2000, NULL);

		m_nState = _ERASE;
	}
	else if (m_nState == _ERASE)
	{
		// program from first page/block 
		m_nActivePage = 0;
		m_nActiveBlock = 0;

		m_nState = _PROGRAM;

		//
		PostMessage(WM_COMMAND, IDC_EXECUTE);
	}
	else if (m_nState == _PROGRAM)
	{
		if (m_nActivePage < m_nTotalPage)
		{
			if (m_nActiveBlock < PROGRAM_COUNT)
			{
				m_pDlgMain->Log(CDownloaderDlg::_INFO, "Program 0x%08X [%3d/%3d]",
							m_nProgramStart + m_nActivePage * PAGE_SIZE, 
							m_nActivePage * (PAGE_SIZE / PROGRAM_SIZE) + (m_nActiveBlock + 1),
							m_nTotalPage * (PAGE_SIZE / PROGRAM_SIZE));
				RequestProgram(m_nProgramStart + m_nActivePage * PAGE_SIZE + m_nActiveBlock * PROGRAM_SIZE);
				SetTimer(TIMER_CHECK_RESPONSE, 2000, NULL);
			}
			else
			{
				// program next page from first block
				m_nActivePage = m_nActivePage + 1;
				m_nActiveBlock = 0;

				PostMessage(WM_COMMAND, IDC_EXECUTE);
			}
		}
		else
		{
			if (m_bVerifyProgram)
			{
				// verify from first page/block 
				m_nActivePage = 0;
				m_nActiveBlock = 0;

				m_nState = _VERIFY;
			}
			else
			{
				// OK! we complete all
				m_nState = _DONE;
			}

			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
	}
	else if (m_nState == _VERIFY)
	{
		if (m_nActivePage < m_nTotalPage)
		{
			if (m_nActiveBlock < PROGRAM_COUNT)
			{
				m_pDlgMain->Log(CDownloaderDlg::_INFO, "Verify 0x%08X [%3d/%3d]",
					m_nProgramStart + m_nActivePage * PAGE_SIZE,
					m_nActivePage * (PAGE_SIZE / PROGRAM_SIZE) + (m_nActiveBlock + 1),
					m_nTotalPage * (PAGE_SIZE / PROGRAM_SIZE));
				RequestVerify(m_nProgramStart + m_nActivePage * PAGE_SIZE + m_nActiveBlock * PROGRAM_SIZE);
				SetTimer(TIMER_CHECK_RESPONSE, 2000, NULL);
			}
			else
			{
				// verify next page from first block
				m_nActivePage = m_nActivePage + 1;
				m_nActiveBlock = 0;

				PostMessage(WM_COMMAND, IDC_EXECUTE);
			}
		}
		else
		{
			// OK! we complete all
			m_nState = _DONE;

			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
	}
	else if (m_nState == _DONE)
	{
		m_pDlgMain->Log(CDownloaderDlg::_INFO, "Complete programming!");

		if (m_bRunAfterProgram)
			RequestRun();
		else
			AfxMessageBox("Done!!", MB_ICONINFORMATION);

		CDialogEx::EndDialog(m_bRunAfterProgram ? IDYES : IDOK);
	}
}

void CProgramDlg::OnBPacketReceived(BPacket * pPacket)
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
			PostMessage(WM_DEVICE_ERROR, pPacket->e.error);
		}
	}
	else if (m_nState == _PROGRAM)
	{
		if (pPacket->code == DCODE_ACK)
		{
			// program success
			m_nActiveBlock += 1;

			// continue
			PostMessage(WM_COMMAND, IDC_EXECUTE);
		}
		else if (pPacket->code == DCODE_NACK)
		{
			// error handling
			PostMessage(WM_DEVICE_ERROR, pPacket->e.error);
		}
	}
	else if (m_nState == _VERIFY)
	{
		if (pPacket->code == DCODE_DUMP_MEM)
		{
			// compare program data with read data
			uint32_t addr = /*m_nProgramStart +*/ m_nActivePage * PAGE_SIZE + m_nActiveBlock * PROGRAM_SIZE;
			int i;

			for (i = 0; i < PROGRAM_SIZE; i++)
				if (m_pFirmware[addr + i] != pPacket->d.data[i])
					break; // OOPS!!

			if (i < PROGRAM_SIZE)
			{
				// error handling
				PostMessage(WM_DEVICE_ERROR, ERROR_MEMORY_COLLAPSE);
			}
			else
			{
				// verfiy success
				m_nActiveBlock += 1;

				// continue
				PostMessage(WM_COMMAND, IDC_EXECUTE);
			}
		}
		else if (pPacket->code == DCODE_NACK)
		{
			// error handling
			PostMessage(WM_DEVICE_ERROR, pPacket->e.error);
		}
	}
}

void CProgramDlg::OnTimer(UINT nEventID)
{	
	CDialogEx::OnTimer(nEventID);

	if (nEventID == m_nTimerID)
	{
		// error handling : timeout
		PostMessage(WM_DEVICE_ERROR, ERROR_RESPONSE_TIMEOUT);

		//
		KillTimer(m_nTimerID);
		m_nTimerID;
	}
}

LPCTSTR CProgramDlg::GetErrorString(uint16_t error)
{
	switch (error)
	{
	case ERROR_OK				: // (0)
		return "OK";
	case ERROR_GENERIC			: // (0x8000)
		return "Generic";
	case ERROR_OUT_OF_MEMORY	: // (0x8001)
		return "Out of Memory";
	case ERROR_FLASH_BUSY		: // (0x8002)
		return "Flash Busy";
	case ERROR_FLASH_ERROR_PG	: // (0x8003)
		return "Flash Proggram Error";
	case ERROR_FLASH_ERROR_WRP	: // (0x8004)
		return "Flash Write Protect Error";
	case ERROR_FLASH_TIMEOUT	: // (0x8005)
		return "Flash Timeout";
	case ERROR_MEMORY_COLLAPSE	: // (0xC001)
		return "Flash Memory Collapse";
	case ERROR_RESPONSE_TIMEOUT	: // (0xC002)
		return "Response Timeout";
	case ERROR_INVALID_PARAM	: // (0xC003)
		return "Invalid Parameter";
	}

	return "Unknown";
}

LRESULT CProgramDlg::OnError(WPARAM wParam, LPARAM lParam)
{
	//
	CString str;
	str.Format(_T("Programming Error(%04X) : %s"), wParam, GetErrorString(wParam));
	m_pDlgMain->Log(CDownloaderDlg::_INFO, str);
	AfxMessageBox(str, MB_ICONSTOP | MB_OK);

	// 
	m_nState = _READY;
	EnableControls();

	return 0;
}