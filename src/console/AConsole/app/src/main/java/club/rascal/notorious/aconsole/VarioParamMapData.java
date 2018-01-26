package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-26.
 */

public class VarioParamMapData {
    public int mParam;
    public int mPrefName;
    public int mType;
    public String mDefault;

    public VarioParamMapData(int param, int name, int type, String def) {
        mParam = param;
        mPrefName = name;
        mType = type;
        mDefault = def;
    }

    public static final int TYPE_BOOLEAN = 0;
    public static final int TYPE_INTEGER = 1;
    public static final int TYPE_LONG = 2;
    public static final int TYPE_FLOAT = 3;
    public static final int TYPE_DOUBLE = 4;
    public static final int TYPE_STRING = 5;
}