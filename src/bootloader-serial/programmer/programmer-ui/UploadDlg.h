#pragma once

#include "Packet.h"

////////////////////////////////////////////////////////////////////////////////////
//

class CDownloaderDlg;


////////////////////////////////////////////////////////////////////////////////////
//

class CUploadDlg : public CDialogEx, protected PacketListener
{
public:
	CUploadDlg(CDownloaderDlg * pParent);
	~CUploadDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRAM };
#endif

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void		OnPacketReceived(PACKET * pPacket);

	BOOL				UpdateData(BOOL bSaveAndValidate = TRUE);

private:
	void				RequestMemory(uint32_t address, uint16_t size);

protected:
	//
	UINT				m_nTimerID;

	//
	CDownloaderDlg *	m_pDlgMain;

protected:
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();

	afx_msg void		OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};

