package com.reactnativejsicpr;

import android.content.Context;
import android.util.Log;

import com.facebook.react.bridge.ReactApplicationContext;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;

public class CertUtils {
  private static final String CERTIFICATE_BUNDLE_FILENAME = "certificate_bundle";
  static File certificateBundle(ReactApplicationContext context) {
    return new File(context.getFilesDir().getAbsolutePath().concat(File.separator + CERTIFICATE_BUNDLE_FILENAME));
  }

  private static boolean traverse(File dir, ReactApplicationContext context) {
    boolean result = false;

    if (dir.exists()) {
      File[] files = dir.listFiles();
      if (files == null) return false;
      long sz = 0;
      for (File file : files) {
        if (file.isDirectory()) {
          return traverse(file, context);
        } else {
          BufferedReader brin = null;
          BufferedWriter brout = null;
          try {
            long fsz = file.length();

            sz += fsz;
            FileInputStream fin = new FileInputStream(file);
            FileOutputStream fout = context.openFileOutput(CERTIFICATE_BUNDLE_FILENAME, Context.MODE_APPEND);
            brin = new BufferedReader(new InputStreamReader(fin, StandardCharsets.UTF_8));
            brout = new BufferedWriter(new OutputStreamWriter(fout, StandardCharsets.UTF_8));
            String line;
            while ((line = brin.readLine()) != null) {
              brout.write(line + "\n");
            }
            result = true;
          } catch (IOException e) {
            e.printStackTrace();
          } finally {
            try {
              if (brin != null)
                brin.close();

              if (brout != null)
                brout.close();
            } catch (IOException e) {
              e.printStackTrace();
              result = false;
            }
          }
        }
      }
      Log.d("WsHelper", "sz: " + String.valueOf(sz));
    }

    return result;
  }

  static boolean writeCertificateBundle(ReactApplicationContext context, File certificateBundle) {
    String ANDROID_ROOT = System.getenv("ANDROID_ROOT");
    File CA_CERTS_DIR_SYSTEM = new File(ANDROID_ROOT + "/etc/security/cacerts");
    if (!CA_CERTS_DIR_SYSTEM.exists() || !CA_CERTS_DIR_SYSTEM.isDirectory()) return false;
    if (certificateBundle.isFile() && certificateBundle.exists()) certificateBundle.delete();
    return traverse(CA_CERTS_DIR_SYSTEM, context);
  }
}
