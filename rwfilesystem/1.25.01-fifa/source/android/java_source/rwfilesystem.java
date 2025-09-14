package com.ea.rwfilesystem;
import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Environment;
import java.io.File;

public class rwfilesystem
{
    /**
     * Before using any EAIO application package files it is necessary to
     * register EAIO with the Java runtime.  To do so simply call this
     * startup method and pass in the AssetManager for your Activity.
     *
     * For example you may want to call the method from you onCreate method:
     *
     *
     *   import com.ea.rwfilesystem.rwfilesystem;
     *
     *   ...
     *
     *   rwfilesystem.Startup(this);    // When called from the Activity onCreate method.
     *
     * @see Shutdown
     */
    public static void Startup(Activity activity)
    {
        File dataRoot = Environment.getDataDirectory();
        File fileRoot = activity.getFilesDir();
        File externalRoot = Environment.getExternalStorageDirectory();
        System.out.println("Java StartupNativeImpl");
        StartupNativeImpl(activity.getAssets(), dataRoot.getAbsolutePath(), fileRoot.getAbsolutePath(), externalRoot.getAbsolutePath());
        System.out.println("Java Ended StartupNativeImpl");
    }


    /**
     * If you previously have called Startup, then the Shutdown method should
     * be called by your application before it exits.  This is necessary to
     * prevent leaking Java objects that were created by C++.
     *
     * @see Startup
     */
    public static native void Shutdown();


    private static native void StartupNativeImpl(AssetManager AssetManager, String dataDir, String filesDir, String externalDir);
}
