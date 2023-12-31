#include "test_flow_postrocess_helper.h"




/**
 * @brief Testfall für Überprüfung allowNegatives
 * 
 */
void testNegative() {


        // Ohne decimal_shift
        std::vector<float> digits = { 1.2, 6.7};
        std::vector<float> analogs = { 9.5, 8.4};
        double preValue_extended = 16.985;
        double preValue = 16.98;
        
        const char* expected = "16.98";
        
        // extendResolution=false
        // da kein negativ, sollte kein Error auftreten
        UnderTestPost* underTestPost = init_do_flow(analogs, digits, Digital100, false, false, 0);
        setAllowNegatives(underTestPost, false);
        setPreValue(underTestPost, preValue);
        std::string result = process_doFlow(underTestPost);
        TEST_ASSERT_EQUAL_STRING("no error", underTestPost->getReadoutError().c_str());
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
        delete underTestPost;

        // extendResolution=true
        // da negativ im Rahmen (letzte Stelle -0.2 > ergebnis), kein Error
        // Aber der PreValue wird gesetzt
        underTestPost = init_do_flow(analogs, digits, Digital100, false, true, 0);
        setAllowNegatives(underTestPost, false);
        setPreValue(underTestPost, preValue_extended);
        result = process_doFlow(underTestPost);
        TEST_ASSERT_EQUAL_STRING("no error", underTestPost->getReadoutError().c_str());
        TEST_ASSERT_EQUAL_STRING(RundeOutput(preValue_extended, analogs.size()+1).c_str(), result.c_str());
        delete underTestPost;

        // extendResolution=true
        // Tolleranz überschritten, Error wird gesetzt, kein ReturnValue
        preValue_extended = 16.988; // zu groß
        underTestPost = init_do_flow(analogs, digits, Digital100, false, true, 0);
        setAllowNegatives(underTestPost, false);
        setPreValue(underTestPost, preValue_extended);
        result = process_doFlow(underTestPost);
        TEST_ASSERT_EQUAL_STRING("Neg. Rate - Read:  - Raw: 16.984 - Pre: 16.988 ", underTestPost->getReadoutError().c_str());
        TEST_ASSERT_EQUAL_STRING("", result.c_str());
        delete underTestPost;

        // extendResolution=false
        // value < (preValue -.01)
        preValue = 17.00; // zu groß
        underTestPost = init_do_flow(analogs, digits, Digital100, false, false, 0);
        setAllowNegatives(underTestPost, false);
        setPreValue(underTestPost, preValue);
        result = process_doFlow(underTestPost);
        TEST_ASSERT_EQUAL_STRING("Neg. Rate - Read:  - Raw: 16.98 - Pre: 17.00 ", underTestPost->getReadoutError().c_str());
        TEST_ASSERT_EQUAL_STRING("", result.c_str());
        delete underTestPost;

        // extendResolution=false
        // value > (preValue -.01)
        // ist im Rahmen der Ungenauigkeit (-1 auf letzter Stelle)
        preValue = 16.99; // zu groß
        underTestPost = init_do_flow(analogs, digits, Digital100, false, false, 0);
        setAllowNegatives(underTestPost, false);
        setPreValue(underTestPost, preValue);
        result = process_doFlow(underTestPost);
        TEST_ASSERT_EQUAL_STRING("no error", underTestPost->getReadoutError().c_str());
        TEST_ASSERT_EQUAL_STRING("16.99", result.c_str());
        delete underTestPost;

        // extendResolution=false
        // value < preValue
        // Aber Prüfung abgeschaltet => kein Fehler
        preValue = 17.99; // zu groß
        underTestPost = init_do_flow(analogs, digits, Digital100, false, false, 0);
        setAllowNegatives(underTestPost, true);
        setPreValue(underTestPost, preValue_extended);
        result = process_doFlow(underTestPost);
        TEST_ASSERT_EQUAL_STRING("no error", underTestPost->getReadoutError().c_str());
        TEST_ASSERT_EQUAL_STRING(expected, result.c_str());
        delete underTestPost;

}

/**
 * @brief Fehlerberichte aus Issues
 * 
 */
void testNegative_Issues() {
        // Ohne decimal_shift
        std::vector<float> digits = { 2.0, 2.0, 0.0, 1.0, 7.2, 9.0, 8.0};
        std::vector<float> analogs = { };
        double preValue_extended = 22018.090;
        double preValue = 22018.09;
        
        const char* expected = "22017.98";

        // https://github.com/jomjol/AI-on-the-edge-device/issues/2145#issuecomment-1461899094
        // extendResolution=false
        // value < preValue
        // Prüfung eingeschaltet => Fehler
        preValue = 22018.09; // zu groß
        UnderTestPost* underTestPost = init_do_flow(analogs, digits, Digital100, false, false, -2);
        setAllowNegatives(underTestPost, false);
        setPreValue(underTestPost, preValue_extended);
        std::string result = process_doFlow(underTestPost);
        TEST_ASSERT_EQUAL_STRING("Neg. Rate - Read:  - Raw: 22017.98 - Pre: 22018.08 ", underTestPost->getReadoutError().c_str());
        // if negativ no result any more

        TEST_ASSERT_EQUAL_STRING("", result.c_str());
        delete underTestPost;

}