package club.rascal.notorious.aconsole;

import android.util.Log;

/**
 * Created by Jasmine on 2018-01-23.
 */

public class VarioResponse {
    public int mCode;           // Command code ex) 'UP', 'QP', 'RS', ...
    public long mParam;

    public int mDataType;
    public int mDataCount;

    public long[] mDataL;
    public double[] mDataD;
    public String mDataS;

    public static final int DTYPE_NONE = 0;
    public static final int DTYPE_NUMBER = 1;
    public static final int DTYPE_FLOAT = 2;
    public static final int DTYPE_STRING = 3;

    /*
    public static final int RCODE_RESULT		    = 'R' * 256 + 'S';
    public static final int RCODE_OK				= 'O' * 256 + 'K';
    public static final int RCODE_FAIL			    = 'F' * 256 + 'A';
    public static final int RCODE_ERROR			= 'E' * 256 + 'R';
    public static final int RCODE_NOT_READY		= 'N' * 256 + 'R';
    public static final int RCODE_UNAVAILABLE		= 'U' * 256 + 'A';
    public static final int RCODE_DUMP_PARAM		= 'D' * 256 + 'P';
    public static final int RCODE_QUERY_PARAM		= 'Q' * 256 + 'P';
    public static final int RCODE_UPDATE_PARAM	= 'U' * 256 + 'P';
    */

    public static final int RPARAM_OK = 0;
    public static final int RPARAM_SUCCESS = 1;
    public static final int RPARAM_FAIL = 2;
    public static final int RPARAM_ERROR = 3;
    public static final int RPARAM_NOT_READY = 4;
    public static final int RPARAM_UNAVAILABLE = 5;

    public static final int MAX_DATACOUNT   = 4;

    private VarioResponse(int code) {
        mCode = code;
        mParam = 0;

        mDataType = DTYPE_NONE;
        mDataCount = 0;

        mDataL = null;
        mDataD = null;
        mDataS = null;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder(32);

        // code
        sb.append('%');
        sb.append((char)(mCode / 256));
        sb.append((char)(mCode % 256));

        // param
        if (mParam != 0) {
            sb.append(',');
            sb.append(mParam);

            if (mDataType == DTYPE_NUMBER) {
                for (int i = 0; i < mDataCount; i++) {
                    sb.append(',');
                    sb.append(mDataL[i]);
                }
            } else if (mDataType == DTYPE_FLOAT) {
                for (int i = 0; i < mDataCount; i++) {
                    sb.append(',');
                    sb.append(mDataD[i]);
                }
            } else if (mDataType == DTYPE_STRING) {
                sb.append(',');
                sb.append(mDataS);
            }
        }

        return sb.toString();
    }
    
    public int getInteger() {
        switch (mDataType) {
            case DTYPE_NUMBER :
                return (int)mDataL[0];
            case DTYPE_FLOAT :
                return (int)mDataD[0];
            case DTYPE_STRING :
                return Integer.parseInt(mDataS);
        }
        
        return 0;
    }
    
    public float getFloat() {
        switch (mDataType) {
            case DTYPE_NUMBER :
                return (float)mDataL[0];
            case DTYPE_FLOAT :
                return (float)mDataD[0];
            case DTYPE_STRING :
                return Float.parseFloat(mDataS);
        }

        return 0;
    }
    
    public String getString() {
        switch (mDataType) {
            case DTYPE_NUMBER :
                return Long.toString(mDataL[0]);
            case DTYPE_FLOAT :
                return Double.toString(mDataD[0]);
            case DTYPE_STRING :
                return mDataS;
        }

        return "";
    }

    //
    public static int verifyType(String str) {
        int type = DTYPE_NONE;

        for (int i = 0; i < str.length(); i++) {
            int c = str.charAt(i);

            if (c >= '0' && c <= '9') {
                if (type == DTYPE_NONE)
                    type = DTYPE_NUMBER;
            }
            else if (c == '.') {
                if (type == DTYPE_NUMBER) {
                    type = DTYPE_FLOAT;
                }
                else {
                    type = DTYPE_STRING;
                    break;
                }
            }
            else if (c == '-') {
                if (i > 0) {
                    type = DTYPE_STRING;
                    break;
                }
                // else ignore -> sign character must be first
            }
            else {
                type = DTYPE_STRING;
                break;
            }
        }

        return type;
    }

    public static VarioResponse parse(String line) {
        // %{CODE}[,{PARAM}[,{VALUE1}[,{VALUE2}[,{VALUE3}[,{VALUE4}]]]]]\r\n
        // %{CODE}[,{PARAM}[,{STRING}]]\r\n
        String[] tokens = line.split(",");
        int tokCount = tokens.length;

        if (tokens[0].length() != 3)
            return null;

        //
        VarioResponse res = new VarioResponse(tokens[0].charAt(1) * 256 + tokens[0].charAt(2));

        if (tokCount > 1) {
            res.mParam = Long.parseLong(tokens[1]);

            if (tokCount > 2) {
                int dataCount = tokCount > 6 ? 4 : tokCount - 2;
                int dataType = verifyType(tokens[2]);

                for (int i = 1; i < dataCount; i++) {
                    if (dataType != verifyType(tokens[2+i])) {
                        dataType = DTYPE_STRING;
                        dataCount = 1;
                        break;
                    }
                }

                res.mDataCount = dataCount; // we treat first value only -> ignore others
                res.mDataType = dataType;

                switch (res.mDataType) {
                    case DTYPE_NUMBER :
                        res.mDataL = new long[res.mDataCount];
                        for (int i = 0; i < dataCount; i++)
                            res.mDataL[i] = Long.parseLong(tokens[2+i]);
                        break;
                    case DTYPE_FLOAT :
                        res.mDataD = new double[res.mDataCount];
                        for (int i = 0; i < dataCount; i++)
                            res.mDataD[i] = Double.parseDouble(tokens[2+i]);
                        break;
                    case DTYPE_STRING :
                        StringBuilder sb = new StringBuilder(32);
                        for (int i = 0; i  < tokCount - 2; i++) {
                            // if string value has comma, ...
                            sb.append(tokens[2+i]);
                        }
                        res.mDataS = sb.toString();
                        break;
                }
            }
        }

        return res;
    }
}