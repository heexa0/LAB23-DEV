#include <jni.h>
#include <string>
#include <algorithm>
#include <climits>
#include <android/log.h>
#include <chrono>
#include <set>

#define LOG_TAG "JNI_DEMO"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

/* ---- hello ---- */
static jstring fn_hello(JNIEnv* env, jobject) {
    LOGI("fn_hello appelé");
    return env->NewStringUTF("Bonjour depuis le code natif C++ !");
}

/* ---- factorielle ---- */
static jint fn_factorial(JNIEnv* env, jobject, jint n) {
    if (n < 0) {
        LOGE("entrée négative refusée");
        return -1;
    }
    long long produit = 1;
    for (int k = 2; k <= n; ++k) {
        produit *= k;
        if (produit > INT_MAX) {
            LOGE("dépassement entier à k=%d", k);
            return -2;
        }
    }
    LOGI("factorial(%d) = %lld", n, produit);
    return static_cast<jint>(produit);
}

/* ---- inversion de chaîne ---- */
static jstring fn_reverse(JNIEnv* env, jobject, jstring input) {
    if (!input) return env->NewStringUTF("[null]");
    const char* raw = env->GetStringUTFChars(input, nullptr);
    if (!raw)   return env->NewStringUTF("[erreur]");
    std::string buf(raw);
    env->ReleaseStringUTFChars(input, raw);
    std::reverse(buf.begin(), buf.end());
    LOGI("inversion => %s", buf.c_str());
    return env->NewStringUTF(buf.c_str());
}

/* ---- somme de tableau ---- */
static jint fn_sum(JNIEnv* env, jobject, jintArray arr) {
    if (!arr) { LOGE("tableau nul"); return -1; }
    jsize len   = env->GetArrayLength(arr);
    jint* elems = env->GetIntArrayElements(arr, nullptr);
    if (!elems) return -2;
    long long total = 0;
    for (jsize i = 0; i < len; ++i) total += elems[i];
    env->ReleaseIntArrayElements(arr, elems, 0);
    if (total > INT_MAX) { LOGE("dépassement somme"); return -3; }
    LOGI("somme = %lld", total);
    return static_cast<jint>(total);
}

/* ---- multiplication de matrices ---- */
static jintArray fn_matmul(JNIEnv* env, jobject,
                            jintArray A, jintArray B, jint N) {
    jint* a = env->GetIntArrayElements(A, nullptr);
    jint* b = env->GetIntArrayElements(B, nullptr);
    jintArray C = env->NewIntArray(N * N);
    jint* c     = env->GetIntArrayElements(C, nullptr);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            long long s = 0;
            for (int k = 0; k < N; ++k) s += a[i*N+k] * b[k*N+j];
            c[i*N+j] = static_cast<jint>(s);
        }

    env->ReleaseIntArrayElements(A, a, 0);
    env->ReleaseIntArrayElements(B, b, 0);
    env->ReleaseIntArrayElements(C, c, 0);
    return C;
}

/* ---- validation de caractères ---- */
static jboolean fn_check(JNIEnv* env, jobject, jstring input) {
    if (!input) return JNI_TRUE;
    const char* raw = env->GetStringUTFChars(input, nullptr);
    std::string s(raw);
    env->ReleaseStringUTFChars(input, raw);
    std::set<char> interdits = {'<', '>', '"', '\'', ';', '&', '|', '`'};
    for (char ch : s)
        if (interdits.count(ch)) {
            LOGE("caractère interdit détecté : %c", ch);
            return JNI_TRUE;
        }
    return JNI_FALSE;
}

/* ---- benchmark natif ---- */
static jlong fn_benchmark(JNIEnv* env, jobject, jint iterations) {
    auto debut = std::chrono::high_resolution_clock::now();
    volatile long long x = 0;
    for (int i = 1; i <= iterations; ++i) x += i;
    auto fin = std::chrono::high_resolution_clock::now();
    jlong ns = std::chrono::duration_cast<std::chrono::nanoseconds>(fin - debut).count();
    LOGI("benchmark natif : %lld ns", (long long)ns);
    return ns;
}

/* ---- table d'enregistrement JNI ---- */
static const JNINativeMethod kTable[] = {
        {"helloFromJNI",   "()Ljava/lang/String;",                  (void*)fn_hello},
        {"factorial",      "(I)I",                                   (void*)fn_factorial},
        {"reverseString",  "(Ljava/lang/String;)Ljava/lang/String;", (void*)fn_reverse},
        {"sumArray",       "([I)I",                                  (void*)fn_sum},
        {"matMul",         "([I[II)[I",                              (void*)fn_matmul},
        {"hasInvalidChars","(Ljava/lang/String;)Z",                  (void*)fn_check},
        {"nativeBenchmark","(I)J",                                   (void*)fn_benchmark},
};

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) return -1;
    jclass clazz = env->FindClass("com/example/jnidemo/MainActivity");
    if (!clazz) return -1;
    env->RegisterNatives(clazz, kTable, sizeof(kTable)/sizeof(kTable[0]));
    return JNI_VERSION_1_6;
}
