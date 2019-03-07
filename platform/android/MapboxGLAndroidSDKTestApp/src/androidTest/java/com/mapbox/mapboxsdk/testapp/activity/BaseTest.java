package com.mapbox.mapboxsdk.testapp.activity;

import android.support.test.espresso.ViewInteraction;
import android.support.test.rule.ActivityTestRule;
import com.mapbox.mapboxsdk.Mapbox;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.testapp.R;
import com.mapbox.mapboxsdk.testapp.action.MapboxMapAction;
import com.mapbox.mapboxsdk.testapp.action.WaitAction;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.rules.TestName;
import timber.log.Timber;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.matcher.ViewMatchers.withId;
import static junit.framework.TestCase.assertNotNull;
import static junit.framework.TestCase.assertTrue;

/**
 * Base class for all Activity test hooking into an existing Activity that will load style.
 */
public abstract class BaseTest implements MapView.OnDidFinishLoadingStyleListener {


  @Rule
  public ActivityTestRule rule = new ActivityTestRule<>(getActivityClass());

  @Rule
  public TestName testName = new TestName();

  protected MapboxMap mapboxMap;
  protected MapView mapView;

  private CountDownLatch latch = new CountDownLatch(1);
  private static int timeoutCount;

  @Before
  public void beforeTest() {
    try {
      setupMap();
      holdTestExecution();
    } catch (Throwable throwable) {
      throwable.printStackTrace();
    }
  }

  private void setupMap() throws Throwable {
    rule.runOnUiThread(() -> {
      mapView = rule.getActivity().findViewById(R.id.mapView);
      mapView.addOnDidFinishLoadingStyleListener(this);
      mapView.getMapAsync(this::initMap);
    });
  }

  private void holdTestExecution() throws InterruptedException {
    boolean success = latch.await(30, TimeUnit.SECONDS);
    if (!success) {
      Timber.e("Countdown latch timeout occurred for %s", testName.getMethodName());
      timeoutCount++;
    }
  }

  @Override
  public void onDidFinishLoadingStyle() {
    latch.countDown();
  }

  protected void initMap(MapboxMap mapboxMap) {
    this.mapboxMap = mapboxMap;

    Style style = mapboxMap.getStyle();
    if (style != null && style.isFullyLoaded()) {
      onDidFinishLoadingStyle();
    }
  }

  protected void validateTestSetup() {
    if (!Mapbox.isConnected()) {
      Timber.e("Not connected to the internet while running test");
    }

    assertTrue("Failing test suite, too many timeouts", timeoutCount < 2);
    assertNotNull("MapView isn't initialised", mapView);
    assertNotNull("MapboxMap isn't initialised", mapboxMap);
    assertNotNull("Style isn't initialised", mapboxMap.getStyle());
    assertTrue("Style isn't fully loaded", mapboxMap.getStyle().isFullyLoaded());
  }

  protected MapboxMap getMapboxMap() {
    return mapboxMap;
  }

  protected abstract Class getActivityClass();

  protected void waitAction() {
    waitAction(500);
  }

  protected void waitAction(long waitTime) {
    onView(withId(R.id.mapView)).perform(new WaitAction(waitTime));
  }

  protected ViewInteraction onMapView() {
    return onView(withId(R.id.mapView));
  }

  protected MapboxMapAction getMapboxMapAction(MapboxMapAction.OnInvokeActionListener onInvokeActionListener) {
    return new MapboxMapAction(onInvokeActionListener, mapboxMap);
  }

  @After
  public void afterTest() {
    mapView.removeOnDidFinishLoadingStyleListener(this);
  }
}

