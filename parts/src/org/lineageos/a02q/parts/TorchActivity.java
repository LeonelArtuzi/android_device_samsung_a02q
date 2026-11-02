/*
 * Torch intensity. The level goes to persist.sys.a02q.torch_level, which the
 * camera shim applies whenever it lights the rear torch. Not re-applied to a
 * lit torch: enabling it from here would make this process the torch owner,
 * and CameraService turns the torch off when the owner dies.
 */
package org.lineageos.a02q.parts;

import android.os.Bundle;
import android.os.SystemProperties;

import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.SeekBarPreference;

import com.android.settingslib.collapsingtoolbar.CollapsingToolbarBaseActivity;

public class TorchActivity extends CollapsingToolbarBaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (savedInstanceState == null) {
            getSupportFragmentManager().beginTransaction()
                    .replace(com.android.settingslib.collapsingtoolbar.R.id.content_frame,
                            new TorchFragment())
                    .commit();
        }
    }

    public static class TorchFragment extends PreferenceFragmentCompat {
        private static final String PROP = "persist.sys.a02q.torch_level";
        private static final int STOCK_LEVEL = 4;

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            setPreferencesFromResource(R.xml.torch_settings, rootKey);
            SeekBarPreference level = findPreference("torch_level");
            level.setValue(SystemProperties.getInt(PROP, STOCK_LEVEL));
            level.setOnPreferenceChangeListener((pref, value) -> {
                SystemProperties.set(PROP, String.valueOf(value));
                return true;
            });
        }
    }
}
