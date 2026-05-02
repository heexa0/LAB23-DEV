package com.example.jnidemo;

import android.util.Log;

/**
 * Gestionnaire de sécurité native.
 * Centralise toutes les vérifications anti-debug et anti-instrumentation,
 * en les isolant du reste de l'application.
 */
public class NativeSecurityManager {

    private static final String TAG = "SEC_MANAGER";

    // Codes représentant l'état de l'environnement d'exécution
    public static final int STATUS_OK              = 0;
    public static final int STATUS_TRACE_DETECTED  = 1;
    public static final int STATUS_MAPS_SUSPICIOUS = 2;
    public static final int STATUS_BOTH_SIGNALS    = 3;

    static {
        System.loadLibrary("security-lib");
    }

    // Méthodes natives implémentées côté C++
    public native boolean isTracedNative();
    public native boolean isMapsContaminated();
    public native int     getSecurityStatus();

    /**
     * Traduit le code numérique en libellé lisible.
     */
    public String getStatusLabel() {
        int code = getSecurityStatus();
        switch (code) {
            case STATUS_OK:
                return "Environnement sain (code 0)";
            case STATUS_TRACE_DETECTED:
                return "Processus sous trace active (code 1)";
            case STATUS_MAPS_SUSPICIOUS:
                return "Bibliothèque d'instrumentation détectée (code 2)";
            case STATUS_BOTH_SIGNALS:
                return "Trace + instrumentation détectées (code 3)";
            default:
                return "Code inconnu (" + code + ")";
        }
    }

    /**
     * Indique si l'environnement peut être considéré comme fiable.
     */
    public boolean isEnvironmentSafe() {
        int code = getSecurityStatus();
        Log.i(TAG, "isEnvironmentSafe() → code=" + code);
        return code == STATUS_OK;
    }

    /**
     * Retourne la couleur d'affichage associée au niveau de menace.
     */
    public int getStatusColor() {
        int code = getSecurityStatus();
        switch (code) {
            case STATUS_OK:              return 0xFF2E7D32; // vert
            case STATUS_TRACE_DETECTED:  return 0xFFE53935; // rouge
            case STATUS_MAPS_SUSPICIOUS: return 0xFFFF6F00; // orange
            case STATUS_BOTH_SIGNALS:    return 0xFF880E4F; // bordeaux
            default:                     return 0xFF616161; // gris neutre
        }
    }
}
