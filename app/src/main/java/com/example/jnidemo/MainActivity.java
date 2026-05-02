package com.example.jnidemo;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Déclarations des méthodes natives (native-lib)
    public native String  helloFromJNI();
    public native int     factorial(int n);
    public native String  reverseString(String s);
    public native int     sumArray(int[] values);
    public native int[]   matMul(int[] A, int[] B, int N);
    public native boolean hasInvalidChars(String s);
    public native long    nativeBenchmark(int iterations);

    static { System.loadLibrary("native-lib"); }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Récupération des vues liées à la sécurité
        TextView lblGlobalStatus = findViewById(R.id.tvSecStatus);
        TextView lblStatusInfo   = findViewById(R.id.tvSecDetail);
        TextView lblPtraceResult = findViewById(R.id.tvTraceCheck);
        TextView lblMapsResult   = findViewById(R.id.tvMapsCheck);

        // Récupération des vues liées aux fonctions JNI
        TextView lblHello   = findViewById(R.id.tvHello);
        TextView lblFacto   = findViewById(R.id.tvFact);
        TextView lblInverse = findViewById(R.id.tvReverse);
        TextView lblSomme   = findViewById(R.id.tvArray);
        TextView lblMat     = findViewById(R.id.tvMatrix);
        TextView lblValide  = findViewById(R.id.tvCheck);
        TextView lblPerf    = findViewById(R.id.tvBench);

        // Création du gestionnaire de sécurité et récupération du statut
        NativeSecurityManager guardian = new NativeSecurityManager();
        int    statusCode  = guardian.getSecurityStatus();
        String statusText  = guardian.getStatusLabel();
        int    statusColor = guardian.getStatusColor();

        // Affichage du résultat global
        lblGlobalStatus.setText("Statut de l'environnement : " + statusText);
        lblGlobalStatus.setTextColor(statusColor);

        // Résultats individuels des deux vérifications
        boolean traceActive  = guardian.isTracedNative();
        boolean mapsInfected = guardian.isMapsContaminated();

        lblPtraceResult.setText("Vérif. ptrace     : " + (traceActive  ? "ALERTE ⚠" : "Sain ✓"));
        lblPtraceResult.setTextColor(traceActive  ? 0xFFE53935 : 0xFF2E7D32);

        lblMapsResult.setText("Vérif. /proc/maps  : " + (mapsInfected ? "ALERTE ⚠" : "Sain ✓"));
        lblMapsResult.setTextColor(mapsInfected ? 0xFFE53935 : 0xFF2E7D32);

        // Message d'explication détaillé
        lblStatusInfo.setText(composeExplanation(statusCode));

        // Exécution des fonctions JNI seulement si l'environnement est fiable
        if (guardian.isEnvironmentSafe()) {

            lblHello.setText(helloFromJNI());

            int result = factorial(7);
            lblFacto.setText(result >= 0
                    ? "Factorielle de 7 = " + result
                    : "Erreur factorielle : code " + result);

            lblInverse.setText("Chaîne inversée : " + reverseString("Android JNI"));

            int[] tableau = {5, 15, 25, 35, 45};
            lblSomme.setText("Somme de {5,15,25,35,45} = " + sumArray(tableau));

            int taille = 2;
            int[] matA = {1, 2, 3, 4};
            int[] matB = {5, 6, 7, 8};
            int[] matC = matMul(matA, matB, taille);
            lblMat.setText(
                    "Résultat A×B =\n"
                    + "[" + matC[0] + ", " + matC[1] + "]\n"
                    + "[" + matC[2] + ", " + matC[3] + "]"
            );

            String cible = "hello<world";
            lblValide.setText("\"" + cible + "\" — caractères illicites : "
                    + hasInvalidChars(cible));

            final int NB_ITER = 1_000_000;
            long tempsNatif = nativeBenchmark(NB_ITER);
            long debut = System.nanoTime();
            long acc = 0;
            for (int i = 1; i <= NB_ITER; i++) acc += i;
            long tempsJava = System.nanoTime() - debut;
            lblPerf.setText("Benchmark " + NB_ITER + " itérations\n"
                    + "Java  : " + tempsJava + " ns\n"
                    + "C++   : " + tempsNatif + " ns");

        } else {
            // Fonctions bloquées — environnement non fiable
            String msg = "[ accès refusé — environnement compromis ]";
            lblHello.setText(msg);
            lblFacto.setText(msg);
            lblInverse.setText(msg);
            lblSomme.setText(msg);
            lblMat.setText(msg);
            lblValide.setText(msg);
            lblPerf.setText(msg);
        }
    }

    /**
     * Retourne un message descriptif en fonction du code de statut reçu.
     */
    private String composeExplanation(int code) {
        switch (code) {
            case NativeSecurityManager.STATUS_OK:
                return "Aucune anomalie relevée. L'accès aux fonctions natives est autorisé.";
            case NativeSecurityManager.STATUS_TRACE_DETECTED:
                return "Un débogueur semble actif sur ce processus (ptrace). "
                     + "L'accès aux fonctions sensibles est restreint.";
            case NativeSecurityManager.STATUS_MAPS_SUSPICIOUS:
                return "Une bibliothèque d'instrumentation a été détectée en mémoire "
                     + "(ex : Frida, Xposed). L'accès est restreint.";
            case NativeSecurityManager.STATUS_BOTH_SIGNALS:
                return "Deux menaces simultanées : débogueur et bibliothèque suspecte. "
                     + "L'accès est entièrement bloqué.";
            default:
                return "Code de statut non reconnu.";
        }
    }
}
