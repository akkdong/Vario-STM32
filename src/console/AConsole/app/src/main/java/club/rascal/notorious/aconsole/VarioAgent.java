package club.rascal.notorious.aconsole;

import android.app.Activity;
import android.app.ActivityManager;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

import app.akexorcist.bluetoothspp.library.BluetoothSPP;
import app.akexorcist.bluetoothspp.library.BluetoothService;
import app.akexorcist.bluetoothspp.library.BluetoothState;


/**
 * Created by Jasmine on 2018-01-23.
 */

public class VarioAgent { // extends BluetoothSPP {
    // VarioAgent is singleton, it coule be created(referensed) by getInstance method
    private static volatile VarioAgent mAgent = new VarioAgent();

    private BluetoothSPP mBluetooth = null;
    private Context mContext = null;
    private int mState = 0;

    private LocationParser locParser = new LocationParser();

    private List<ListenerObject> mListeners = new ArrayList<ListenerObject>();

    // declare listener interface
    public interface VarioListener {
        void onDataReceived(AbstractData data);
        void onResponseReceived(VarioResponse response);
    }

    public class ListenerFilter {
        public static final int FILTER_DATA = 0x0001;
        public static final int FILTER_RESPONSE = 0x0002;
    }

    public class ListenerObject {
        public Activity mActivity;
        public int mFilterMask;
        public VarioListener mListener;

        public ListenerObject(Activity activity, int mask, VarioListener listener) {
            mActivity = activity;
            mFilterMask = mask;
            mListener = listener;
        }
    }

    // singleton private constructor
    private  VarioAgent() { }

    public static VarioAgent getInstance() {
        return mAgent;
    }

    public boolean init(Context context, BluetoothSPP.BluetoothConnectionListener listener) {
        if (mBluetooth == null) {
            //
            mContext = context;

            //
            mBluetooth = new BluetoothSPP(context);
            mBluetooth.setBluetoothConnectionListener(listener);

            mBluetooth.setOnDataReceivedListener(new BluetoothSPP.OnDataReceivedListener() {
                @Override
                public void onDataReceived(byte[] data, String message) {
                    //Log.i("Vario", "VarioAgent.onDataReceived: " + message);

                    if (message.charAt(0) == '$') {
                        AbstractData absData = null;

                        if (message.startsWith("$LK8EX1,")) {
                            absData = VarioParser.parse(message);
                        } else if (message.startsWith("$GPRMC,")) {
                            locParser.parse(message, LocationParser.NMEA_GPRMC);
                            if (locParser.isReady()) {
                                absData = locParser.getData();
                            }
                        } else if (message.startsWith(("$GPGGA,"))) {
                            locParser.parse(message, LocationParser.NMEA_GPGGA);
                            if (locParser.isReady())
                                absData = locParser.getData();
                        } else if (message.startsWith(("$SENSOR,"))) {
                            absData = SensorParser.parse(message);
                        }

                        if (absData != null) {
                            for (ListenerObject obj : mListeners) {
                                // ?? check validation of object.mActivity
                                // ...

                                if ((obj.mFilterMask & ListenerFilter.FILTER_DATA) == ListenerFilter.FILTER_DATA)
                                    obj.mListener.onDataReceived(absData);
                            }
                        }
                    } else if (message.charAt(0) == '%') {
                        VarioResponse res = VarioResponse.parse(message);

                        if (res != null) {
                            for (ListenerObject obj : mListeners) {
                                // ?? check validation of object.mActivity
                                // ...

                                if ((obj.mFilterMask & ListenerFilter.FILTER_RESPONSE) == ListenerFilter.FILTER_RESPONSE)
                                    obj.mListener.onResponseReceived(res);
                            }
                        }
                    }
                }
            });

            return true;
        }

        return false;
    }

    public boolean isBluetoothAvailable() {
        if (mBluetooth != null && mBluetooth.isBluetoothAvailable())
            return true;

        return false;
    }

    public boolean isBluetoothEnabled() {
        if (mBluetooth != null && mBluetooth.isBluetoothEnabled())
            return true;

        return false;
    }

    public boolean isServiceAvailable() {
        if (mBluetooth != null && mBluetooth.isServiceAvailable())
            return true;

        return false;
    }

    public boolean startService() {
        if (mBluetooth != null) {
            mBluetooth.setupService();
            mBluetooth.startService(false); // BluetoothState.DEVICE_OTHER

            return true;
        }

        return false;
    }

    public boolean stopService() {
        if (mBluetooth != null) {
            mBluetooth.stopService();
            return true;
        }

        return false;
    }

    public BluetoothAdapter getBluetoothAdapter() {
        if (mBluetooth != null) {
            return mBluetooth.getBluetoothAdapter();
        }

        return null;
    }

    public BluetoothSPP getBluetoothSPP() {
        return mBluetooth;
    }

    public int getServiceState() {
        if (mBluetooth != null) {
            return mBluetooth.getServiceState();
        }

        return -1;
    }

    public void setVarioListener(Activity activity, int mask, VarioListener listener) {
        unsetVarioListener(activity); // remove if is exist
        mListeners.add(new ListenerObject(activity, mask, listener));
    }

    public void unsetVarioListener(Activity activity) {
        for (ListenerObject obj : mListeners) {
            if (obj.mActivity == activity) {
                mListeners.remove(obj);
                break;
            }
        }
    }

    public boolean connect(Intent intent) {
        if (mBluetooth != null) {
            mBluetooth.connect(intent);
            return true;
        }

        return false;
    }

    public boolean connect(String address) {
        if (mBluetooth != null) {
            mBluetooth.connect(address);
            return true;
        }

        return false;
    }

    public boolean disconnect() {
        if (mBluetooth != null) {
            mBluetooth.disconnect();
            return true;
        }

        return false;
    }

    public boolean send(VarioCommand command) {
        if (mBluetooth != null) {
            Log.i("Vario", "Send command : " + command.toString());
            mBluetooth.send(command.toString(), true);

            return true;
        }

        return false;
    }

    public boolean send(byte[] data, boolean CRLF) {
        if (mBluetooth != null) {
            mBluetooth.send(data, CRLF);
            return true;
        }

        return false;
    }

    public boolean send(String data, boolean CRLF) {
        if (mBluetooth != null) {
            mBluetooth.send(data, CRLF);
            return true;
        }

        return false;
    }

    protected Boolean isActivityRunning(Class activityClass)
    {
        ActivityManager activityManager = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> tasks = activityManager.getRunningTasks(Integer.MAX_VALUE);

        for (ActivityManager.RunningTaskInfo task : tasks) {
            if (activityClass.getCanonicalName().equalsIgnoreCase(task.baseActivity.getClassName()))
                return true;
        }

        return false;
    }
}
