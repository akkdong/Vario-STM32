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

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRAM };
#endif

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
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

