package club.rascal.notorious.aconsole;

import android.util.Log;

/**
 * Created by Jasmine on 2018-01-23.
 */

public class VarioCommand {
    public int mCode;           // Command code ex) 'UP', 'QP', 'RS', ...
    public int mParam;

    public int mDataType;
    public long mDataL;
    public double mDataD;
    public String mDataS;

    public static final int DTYPE_NONE = 0;
    public static final int DTYPE_NUMBER = 1;
    public static final int DTYPE_FLOAT = 2;
    public static final int DTYPE_STRING = 3;

    public static final int  CMD_STATUS				= 'S' * 256 + 'T';
    public static final int  CMD_RESET				    = 'R' * 256 + 'S';
    public static final int  CMD_SHUTDOWN			    = 'S' * 256 + 'D';
    public static final int  CMD_FIRMWARE_VERSION   	= 'F' * 256 + 'V';
    public static final int  CMD_MODE_SWITCH		    = 'M' * 256 + 'S';
    public static final int  CMD_SOUND_LEVEL		    = 'S' * 256 + 'L';
    public static final int  CMD_TONE_TEST			    = 'T' * 256 + 'T';
    public static final int  CMD_DUMP_SENSOR		    = 'D' * 256 + 'S';
    public static final int  CMD_DUMP_PROPERTY	    = 'D' * 256 + 'P';
    public static final int  CMD_DUMP_CONFIG		    = 'D' * 256 + 'C';
    public static final int  CMD_BLOCK_GPS_NMEA		= 'B' * 256 + 'G';
    public static final int  CMD_BLOCK_VARIO_NMEA	    = 'B' * 256 + 'V';
    public static final int  CMD_FACTORY_RESET		= 'F' * 256 + 'R';
    public static final int CMD_RESTORE_PROPERTY      = 'R' * 256 + 'P';
    public static final int CMD_SAVE_PROPERTY         = 'S' * 256 + 'P';
    public static final int CMD_QUERY_PROPERTY        = 'Q' * 256 + 'P';
    public static final int CMD_UPDATE_PROPERTY       = 'U' * 256 + 'P';

    public VarioCommand(int code) {
        mCode = code;
        mParam = 0;

        mDataType = DTYPE_NONE;
        mDataL = 0;
        mDataD = 0;
        mDataS = null;
    }

    public VarioCommand(int code, int param) {
        mCode = code;
        mParam = param;

        mDataType = DTYPE_NONE;
        mDataL = 0;
        mDataD = 0;
        mDataS = null;
    }

    public VarioCommand(int code, int param, long data) {
        mCode = code;
        mParam = param;

        mDataType = DTYPE_NUMBER;
        mDataL = data;
        mDataD = data;
        mDataS = null;
    }

    public VarioCommand(int code, int param, double data) {
        mCode = code;
        mParam = param;

        mDataType = DTYPE_FLOAT;
        mDataL = (long)data;
        mDataD = data;
        mDataS = null;
    }

    public VarioCommand(int code, int param, String data) {
        mCode = code;
        mParam = param;

        mDataType = DTYPE_STRING;
        mDataL = 0;
        mDataD = 0;
        mDataS = data;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder(32);

        // code
        sb.append('#');
        sb.append((char)(mCode / 256));
        sb.append((char)(mCode % 256));

        // param
        if (mParam != 0) {
            sb.append(',');
            sb.append(mParam);

            if (mDataType != DTYPE_NONE)
                sb.append(',');

            if (mDataType == DTYPE_NUMBER)
                sb.append(mDataL);
            else if (mDataType == DTYPE_FLOAT)
                sb.append(mDataD);
            else if (mDataType == DTYPE_STRING)
                sb.append(mDataS);
        }

        return sb.toString();
    }
}
