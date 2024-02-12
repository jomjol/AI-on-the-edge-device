#include "test_flow_postrocess_helper.h"




/**
 * ACHTUNG! Die Test laufen aktuell nur mit ausgeschaltetem Debug in ClassFlowCNNGeneral 
 * 
 *
 * @brief Testet die doFlow-Methode von ClassFlowPostprocessing
 * digits[] - enthält die liste der vom Model zurückgegebenen Ergebnisse (class100/cont) in der Reihenfolge von links nach rechts
 * analog[] - enthält die Liste der Zeiger vom Model, wie bei den digits
 * expected - enthält das erwartete Ergebnis, wobei der Dezimalpunkt genau zwischen digits und analog ist.
 * 
 */
void test_doFlowPP() {
        
        /*
         * 
         * digit1 = 1.2
         * digit2 = 6.7
         * analog1 = 9.5
         * analog2 = 8.4
         * 
         * Das Ergebnis sollte "16.984" sein. Bzw. 16.98 ohne Extended true
         */
        std::vector<float> digits = { 1.2, 6.7};
        std::vector<float> analogs = { 9.5, 8.4};
        const char* expected = "16.98";
        //const char* expected_extended = "16.984";
        std::string result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921#issue-1344032217
         * 
         * Das Ergebnis sollte "376529.6" sein. 
         */
        digits = { 3.0, 7.0, 6.0, 5.0, 2.5, 9.6};
        analogs = { 6.4};
        expected = "376529.6";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1220365920
         * 
         * Das Ergebnis sollte "167734.6" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 1.1, 6.0, 7.0, 7.0, 3.0, 4.6};
        analogs = { 6.2};
        expected = "167734.6";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/919
         * 
         * Das Ergebnis sollte "58.96889" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 5.0, 8.6};
        analogs = { 9.8, 6.7, 8.9, 8.6, 9.8};
        expected = "58.96889";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
  
        /*
         * https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1222672175
         * 
         * Das Ergebnis sollte "376529.6" sein. Bzw. 16.98 ohne Extended true
         */
        digits = { 2.9, 7.0, 6.8, 9.9, 8.0, 3.9};
        analogs = { 9.7};
        expected = "377083.9";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        digits = { 1.1, 9.0, 4.0};
        analogs = { 6.1, 2.6, 6.25, 9.7};
        expected = "194.6259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        digits = { 1.1, 9.0, 4.0};
        analogs = { 8.1, 2.6, 6.25, 9.7};
        expected = "194.8259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        digits = { 1.1, 9.0, 4.0};
        analogs = { 9.1, 2.6, 6.25, 9.7};
        expected = "194.9259";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/discussions/950
        digits = { 1.0, 9.0, 9.0};
        analogs = { 7.1, 4.8, 8.3};
        expected = "199.748";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/948
        digits = { 1.0, 9.0, 9.0};
        analogs = { 7.1, 4.8, 8.3};
        expected = "199.748";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
       
}

void test_doFlowPP1() {
        // https://github.com/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1226966346
        std::vector<float> digits = { 0.0, 2.9, 3.0, 2.9, 3.5, 9.5};
        std::vector<float>  analogs = {        };
        const char* expected = "33339";
        std::string result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1226966346
        digits = { 9.9, 2.8, 2.9, 2.9, 3.7, 9.7};
        analogs = {        };
        expected = "33339";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/942
        digits = { 0.0, 9.9, 6.8, 9.9, 3.7, 0.8, 6.9, 8.7};
        analogs = {        };
        expected = "704178";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // https://github.com/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1228343319
        digits = { 9.9, 6.8, 1.1, 4.7, 2.7, 6.0, 9.0, 2.8};  // changed 3.7 --> 2.7 (see picture in issue)
        analogs = {        };
        expected = "7153692";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // Analoger Übergang Zähler Jomjolcom/jomjol/AI-on-the-edge-device/issues/942#issuecomment-1228343319
        digits = { 1.0, 9.0, 4.3};  // changed 3.7 --> 2.7 (see picture in issue)
        analogs = { 8.9, 0.7, 8.9, 9.4 };
        expected = "194.9089";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei V11.2.0
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1229552041
        digits = { 2.9, 7.0, 7.0, 9.1, 8.1, 8.5};  // 376.9884(1) als falsches Ergebnis
        analogs = { 4.1 };
        expected = "377988.4";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei V11.2.0
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1233149877
        digits = { 0.0, 0.0, 7.0, 8.9};  // 79.9999(6) als falsches Ergebnis
        analogs = { 0.1, 0.1, 0.1, 9.6};
        expected = "78.9999";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1236119370
        digits = { 3.1, 9.1, 5.7};  // 9.1 führt zu falscher Erkennung eines unvollständigen Übergangs
        analogs = { 8.8, 6.1, 3.0, 2.0};
        expected = "395.8632";
        result = process_doFlow(analogs, digits);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
}

void test_doFlowPP2() {
        // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/discussions/950#discussion-4338615
        std::vector<float> digits = { 1.0, 9.0, 9.0};  // Übergang wurde um 1 erhöht (200, statt 199)
        std::vector<float> analogs = { 7.1, 4.8, 8.3};
        const char* expected = "199.748";
        std::string result = process_doFlow(analogs, digits, Digital);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // Fehler bei Rolling (2002-09-09)
        // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1242730397
        digits = { 3.0, 2.0, 2.0, 8.0, 9.0, 4.0, 1.7, 9.8};  // falscher Wert 32290.420
        analogs = { };
        expected = "32289.419";
        const char* expected_extended= "32289.4198";
        // FALSCH! wegen ungenügender Präzision von NUMBERS->Value
        // expected_extended= "32289.4198";

        // extendResolution=false, checkConsistency=false
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // checkConsistency=true und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // Fehler Rolling (2022-09-10)
        // not documented as issue
        digits = { 0.0, 0.0, 7.9, 3.8};  // 84.99401 als falsches Ergebnis
        analogs = { 0.0, 9.4, 4.1, 0.1};
        expected = "83.9940";
        expected_extended= "83.99401";

        // checkConsistency=false
        result = process_doFlow(analogs, digits, Digital100, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, true);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // checkConsistency=true und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // Fehler Rolling (2022-09-10)
        // https://github.com/jomjol/AI-on-the-edge-device/issues/994#issue-1368570945
        digits = { 0.0, 0.0, 1.0, 2.0, 2.8, 1.9, 2.8, 5.6};  // 123245.6 als falsches Ergebnis
        analogs = { };
        expected = "123235";
        expected_extended= "123235.6";
        
        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, false, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());


        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, true, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/discussions/950#discussioncomment-3661982
        digits = { 3.0, 2.0, 4.1, 9.0, 4.0, 6.3, 9.2};  // 3249.459 als falsches Ergebnis
        analogs = { };
        expected = "3249.469";
        expected_extended= "3249.4692";

        // checkConsistency=true
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler bei V11.2.0 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1020#issue-1375648891
        digits = { 0.0, 2.0, 6.1, 9.2};  // 259.9227 als falsches Ergebnis
        analogs = { 9.0, 2.5, 2.9, 7.2};
        expected = "269.9227";
        expected_extended= "269.92272";
        
        // extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler bei V11.3.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1028#issuecomment-1250239481
        digits = { 1.1, 6.0, 9.1, 3.0, 5.3, 9.4};  // 169.3493 als falsches Ergebnis
        analogs = { 3.5};
        expected = "169.3593";
        expected_extended= "169.35935";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler bei V12.0.1 
        // Lokal
        digits = { 9.8, 9.8, 1.9, 0.9, 0.9, 9.9, 2.9, 4.8};  // 211.0345 als falsches Ergebnis
        analogs = { 5.5};
        expected = "211.0355";
        expected_extended= "211.03555";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler bei V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1277425333
        digits = { 2.2, 4.5, 5.9};  // 245.938 als falsches Ergebnis
        analogs = { 9.4, 3.8, 8.6};
        expected = "245.938";
        expected_extended= "245.9386";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

      // Fehler bei V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1277425333
        digits = { 2.2, 4.5, 5.9};  // 245.938 kein Fehler. Aber Grenzfall, deshalb mit als Test aufgenommen.
        analogs = { 9.4, 3.8, 8.6};
        expected = "245.938";
        expected_extended= "245.9386";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());
}

void test_doFlowPP3() {
        // Fehler bei V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1265523710
        std::vector<float> digits = { 2.0, 4.0, 6.8};  // 246.2045 als falsches Ergebnis
        std::vector<float> analogs = { 2.2, 0.1, 4.5};
        const char* expected = "247.204";
        const char* expected_extended= "247.2045";
        
        // extendResolution=false
        std::string result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        
        // Fehler bei V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issue-1391153343
        digits = { 1.0, 4.0, 2.0};  // 141.9269 als falsches Ergebnis
        analogs = { 9.2, 2.5, 6.8, 9.0};
        expected = "142.9269";
        expected_extended= "142.92690";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());


       // Fehler bei V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1262626388
        digits = { 1.2, 6.8, 0.0, 0.0, 5.0, 2.8};  //170.05387 als falsches Ergebnis
        // letztes digit läuft mit analog zeiger mit. Hier nur lösbar mit setAnalogdigitTransistionStart=7.7
        analogs = { 8.7};
        expected = "170.0528";
        expected_extended= "170.05287";
        
        // extendResolution=false
        UnderTestPost* undertestPost = init_do_flow(analogs, digits, Digital100, false, false, -3);
        setAnalogdigitTransistionStart(undertestPost, 7.7);
        result = process_doFlow(undertestPost);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
        delete undertestPost;

        // checkConsistency=false und extendResolution=true
        undertestPost = init_do_flow(analogs, digits, Digital100, false, true, -3);
        setAnalogdigitTransistionStart(undertestPost, 7.7);
        result = process_doFlow(undertestPost);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());
        delete undertestPost;

        // Fehler bei rolling post V12.0.1 
        // lokal watermeter1
        digits = { 0.0, 0.0, 9.0, 1.0};  //90.88174 als falsches Ergebnis
        analogs = {9.0,  8.0, 1.8, 7.4};
        expected = "91.8817";
        expected_extended= "91.88174";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());


        // Fehler bei rolling post V12.0.1 
        // lokal watermeter1
        digits = { 0.0, 0.0, 9.0, 1.9};  //91.38403 als falsches Ergebnis
        analogs = {3.6,  8.2, 3.2, 2.0};
        expected = "92.3832";
        expected_extended= "92.38320";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // Fehler  V11.3.0 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1143#issue-1400807695
        digits = { 7.0, 4.0, 7.0, 2.0, 7.0, 5.4, 9.4};  // 7472.749 als falsches Ergebnis
        analogs = {};
        expected = "7472.759";
        expected_extended= "7472.7594";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // Fehler  V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1143#issuecomment-1274434805
        digits = { 4.9, 6.9, 6.8};  // 576.8649 als falsches Ergebnis
        analogs = {8.6, 6.2, 5.0, 9.0};
        expected = "577.8649";
        expected_extended= "577.86490";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());


        // Fehler  V12.0.1 "TODO 00211.03480 vs 00211.03580"
        // Lokal "Hängendes Digit"
        digits = { 2.0, 1.0, 1.0, 0.0, 3.0, 4.8};  // 00211.03480 als falsches Ergebnis
        analogs = {8.0};
        expected = "211.0358";
        expected_extended= "211.03580";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler  V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1143#issuecomment-1281231468
        digits = {  1.0, 1.9, 6.0};  // 125.923 als falsches Ergebnis
        analogs = {9.3, 2.3, 3.1};
        expected = "126.923";
        expected_extended= "126.9231";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, 0);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, 0);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

       // Fehler  V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1282168030
        digits = {  3.0, 8.1, 5.9, 0.0, 5.0, 6.7};  // 386.05672 als richtiges Ergebnis. Letztes digit schein mit dem Analogzeiger mitzulaufen
        analogs = {7.2};
        expected = "386.0567";
        expected_extended= "386.05672";
        
        // extendResolution=false
        result = process_doFlow(analogs, digits, Digital100, false, false, -3);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true, -3);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

        // Fehler  V12.0.1 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1282168030
        digits = {  1.2, 7.0, 1.2, 2.0, 4.0, 1.8};  // 171.24278 als falsches Ergebnis. 
        // Test ist nur erfolgreich mit Veränderung des AnalogdigitTransistionStart
        analogs = {7.8};
        expected = "171.2417";
        expected_extended= "171.24178";
        
        // extendResolution=false
        undertestPost = init_do_flow(analogs, digits, Digital100, false, false, -3);
        setAnalogdigitTransistionStart(undertestPost, 7.7);
        result = process_doFlow(undertestPost);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
        delete undertestPost;

        // checkConsistency=false und extendResolution=true
        undertestPost = init_do_flow(analogs, digits, Digital100, false, true, -3);
        setAnalogdigitTransistionStart(undertestPost, 7.7);
        result = process_doFlow(undertestPost);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());
        delete undertestPost;
}

void test_doFlowPP4() {

        // Fehler  V13.0.4 
        // https://github.com/jomjol/AI-on-the-edge-device/issues/1503#issuecomment-1343335855
         std::vector<float> digits = {  0.0, 0.0, 6.9, 1.0, 6.6};  // 716.0199 als falsches Ergebnis. 
        // Test ist nur erfolgreich mit Veränderung des AnalogdigitTransistionStart
         std::vector<float> analogs = {9.9, 1.8, 6.6, 5.8};
        const char* expected = "717.0165";
        const char* expected_extended= "717.01658";
        
        // extendResolution=false
        std::string result = process_doFlow(analogs, digits, Digital100, false, false);
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());

        // checkConsistency=false und extendResolution=true
        result = process_doFlow(analogs, digits, Digital100, false, true);
        TEST_ASSERT_EQUAL_STRING(expected_extended, result.c_str());

}


