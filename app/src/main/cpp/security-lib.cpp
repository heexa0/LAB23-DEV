#include <jni.h>
#include <cstdio>
#include <cstring>
#include <android/log.h>
#include <sys/ptrace.h>
#include <unistd.h>

#define SEC_TAG "SEC_MONITOR"
#define SEC_I(...) __android_log_print(ANDROID_LOG_INFO,  SEC_TAG, __VA_ARGS__)
#define SEC_W(...) __android_log_print(ANDROID_LOG_WARN,  SEC_TAG, __VA_ARGS__)
#define SEC_E(...) __android_log_print(ANDROID_LOG_ERROR, SEC_TAG, __VA_ARGS__)

/*
 * Codes de statut retournés par getSecurityStatus :
 *   0 → environnement sain
 *   1 → traçage ptrace détecté
 *   2 → bibliothèque suspecte dans /proc/self/maps
 *   3 → les deux signaux simultanément
 */

// -----------------------------------------------
// Vérification 1 : débogueur via ptrace
// -----------------------------------------------
static bool detectTracing() {
    long ret = ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    if (ret == -1) {
        SEC_E("[TRACE] ptrace a échoué — processus potentiellement supervisé");
        return true;
    }
    ptrace(PTRACE_DETACH, 0, nullptr, nullptr);
    SEC_I("[TRACE] aucune supervision détectée");
    return false;
}

// -----------------------------------------------
// Vérification 2 : scan de /proc/self/maps
// -----------------------------------------------
static bool detectMapsContamination() {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) {
        SEC_W("[MAPS] impossible d'ouvrir /proc/self/maps");
        return false;
    }

    // Liste des signatures d'outils d'instrumentation connus
    const char* suspects[] = {
            "frida", "libfrida", "frida-agent",
            "xposed", "substrate",
            "gdbserver", "libgdb",
            "magisk",
            "lldb-server",
            nullptr
    };

    char ligne[512];
    while (fgets(ligne, sizeof(ligne), fp)) {
        for (int i = 0; suspects[i] != nullptr; ++i) {
            if (strstr(ligne, suspects[i])) {
                SEC_E("[MAPS] signature suspecte '%s' : %s", suspects[i], ligne);
                fclose(fp);
                return true;
            }
        }
    }
    fclose(fp);
    SEC_I("[MAPS] aucune anomalie détectée dans /proc/self/maps");
    return false;
}

// -----------------------------------------------
// Méthode native : isTracedNative() → boolean
// -----------------------------------------------
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_jnidemo_NativeSecurityManager_isTracedNative(
        JNIEnv*, jobject) {
    return detectTracing() ? JNI_TRUE : JNI_FALSE;
}

// -----------------------------------------------
// Méthode native : isMapsContaminated() → boolean
// -----------------------------------------------
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_jnidemo_NativeSecurityManager_isMapsContaminated(
        JNIEnv*, jobject) {
    return detectMapsContamination() ? JNI_TRUE : JNI_FALSE;
}

// -----------------------------------------------
// Méthode native : getSecurityStatus() → int
// -----------------------------------------------
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_jnidemo_NativeSecurityManager_getSecurityStatus(
        JNIEnv*, jobject) {

    bool trace = detectTracing();
    bool maps  = detectMapsContamination();

    if (trace && maps) {
        SEC_E("[STATUS] code 3 — double menace détectée");
        return 3;
    }
    if (trace) {
        SEC_E("[STATUS] code 1 — trace seule");
        return 1;
    }
    if (maps) {
        SEC_E("[STATUS] code 2 — maps contaminées");
        return 2;
    }
    SEC_I("[STATUS] code 0 — environnement sain");
    return 0;
}
