package club.rascal.notorious.aconsole;

import android.util.Log;

/**
 * Created by Jasmine on 2018-01-24.
 */

public class AbstractData {
    public int mType;

    public static final int DATA_LOCATION = 0x0001;
    public static final int DATA_VARIO = 0x0002;
    public static final int DATA_SENSOR = 0x0003;

    protected AbstractData(int type) {
        mType = type;
    }
}
