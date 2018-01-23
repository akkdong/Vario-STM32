package club.rascal.notorious.aconsole;

import android.app.Activity;
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

    private static volatile VarioAgent agent = new VarioAgent();

    private BluetoothSPP mBluetooth = null;
    private Context mContext = null;
    private int mState = 0;

    private List<ListenerObject> mListeners = new ArrayList<ListenerObject>();

    public interface VarioListener {
        void onGPSUpdated(float latitude, float longigute, float altitude, float speed, float trackAngle);
        void onVarioUpdated(float vario, float pressure, float temperature, float voltage);
        void onAccelerometerUpdated(float ax, float ay, float az);
        void onGyrometerUpdated(float gx, float gy, float gz);
        void onPressureUpdated(float prs);
        void onParameterUpdated();
    }

    public class VarioListenerFilter {
        public static final int FILTER_GPS = 0x0001;
        public static final int FILTER_VARIO = 0x0002;
        public static final int FILTER_ACCELEROMETER = 0x0004;
        public static final int FILTER_GYROMETER = 0x0008;
        public static final int FILTER_PRESSURE = 0x0010;
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

    // private constructor
    private  VarioAgent() { }

    public static VarioAgent getInstance() {
        return agent;
    }

    public boolean init(Context context, BluetoothSPP.BluetoothConnectionListener listener) {
        if (mBluetooth == null) {
            mBluetooth = new BluetoothSPP(context);
            mBluetooth.setBluetoothConnectionListener(listener);

            mBluetooth.setOnDataReceivedListener(new BluetoothSPP.OnDataReceivedListener() {
                @Override
                public void onDataReceived(byte[] data, String message) {
                    Log.i("Vario", message);

                    if (message.startsWith("$LK8")) {
                        for (ListenerObject obj : mListeners) {
                            obj.mListener.onVarioUpdated(0.0f, 0.0f, 0.0f, 0.0f);
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
        for (ListenerObject obj : mListeners) {
            if (obj.mActivity == activity) {
                mListeners.remove(obj);
                break;
            }
        }

        mListeners.add(new ListenerObject(activity, mask, listener));
    }

    public void unsetVarioListener(Activity activity) {
        for (int i = 0; i < mListeners.size(); i++) {
            ListenerObject obj = mListeners.get(i);

            if (obj.mActivity == activity) {
                mListeners.remove(i);
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
}
