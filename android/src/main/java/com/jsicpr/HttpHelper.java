package com.jsicpr;


import static com.jsicpr.CertUtils.certificateBundle;
import static com.jsicpr.CertUtils.writeCertificateBundle;

import com.facebook.jni.HybridData;
import com.facebook.jni.annotations.DoNotStrip;
import com.facebook.react.bridge.JavaScriptContextHolder;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.turbomodule.core.CallInvokerHolderImpl;
import com.facebook.react.turbomodule.core.interfaces.CallInvokerHolder;

import java.io.File;

@SuppressWarnings("JavaJniMissingFunction")
public class HttpHelper {

  @DoNotStrip
  @SuppressWarnings("unused")
  HybridData mHybridData;

  public native HybridData initHybrid(long jsContext, CallInvokerHolderImpl jsCallInvokerHolder);

  public native void installJSIBindings(String certPath);

  public boolean install(ReactApplicationContext context) {
    try {
      JavaScriptContextHolder jsContext = context.getJavaScriptContextHolder();
      CallInvokerHolder jsCallInvokerHolder = context.getCatalystInstance().getJSCallInvokerHolder();
      mHybridData = initHybrid(jsContext.get(), (CallInvokerHolderImpl) jsCallInvokerHolder);

      File certificateBundleFile = certificateBundle(context);
      String path = "";
      if (writeCertificateBundle(context, certificateBundleFile)) {
        path = certificateBundleFile.getAbsolutePath();
      }
      System.out.println("-=== " + path);
      installJSIBindings(path);
      return true;
    } catch (Exception exception) {
      return false;
    }
  }
}
