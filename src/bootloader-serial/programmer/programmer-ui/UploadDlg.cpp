#include "stdafx.h"
#include "resource.h"
#include "UploadDlg.h"
#include "DownloaderDlg.h"


////////////////////////////////////////////////////////////////////////////////////
//

CUploadDlg::CUploadDlg(CDownloaderDlg * pParent)
	: CDialogEx(IDD_UPLOAD, pParent)
	, m_pDlgMain(pParent)
	, m_nTimerID(0)
{
}


CUploadDlg::~CUploadDlg()
{
}


void CUploadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUploadDlg, CDialogEx)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CUploadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	UpdateData(FALSE);

	return TRUE;
}

void CUploadDlg::OnOK()
{
}

void CUploadDlg::OnCancel()
{
	CDialogEx::OnCancel();
}

void CUploadDlg::OnPacketReceived(BPacket * pPacket)
{
//	if (pPacket->code == DCODE_DUMP_MEM)
//		m_wndHexEdit.SetData(pPacket->payloadLen - 4, pPacket->d.data);
}

void CUploadDlg::OnTimer(UINT nEventID)
{
	CDialogEx::OnTimer(nEventID);
}

BOOL CUploadDlg::UpdateData(BOOL bSaveAndValidate)
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

void CUploadDlg::RequestMemory(uint32_t address, uint16_t size)
{
	BPacketMaker maker;

	maker.start(HCODE_READ);
	maker.push_u32(address);
	maker.push_u16(size);
	maker.finish();

	m_pDlgMain->SendCommand(maker.get_data(), maker.get_size());
}
