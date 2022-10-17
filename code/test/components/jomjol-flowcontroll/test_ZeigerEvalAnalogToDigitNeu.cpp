#include <unity.h>
#include <ClassFlowCNNGeneral.h>

class UnderTestCNNGeneral : public ClassFlowCNNGeneral {
    public:
        UnderTestCNNGeneral( ClassFlowAlignment *_flowalign, t_CNNType _cnntype) :
            ClassFlowCNNGeneral(_flowalign, _cnntype) {};
        
        using ClassFlowCNNGeneral::ZeigerEvalAnalogToDigitNeu;
       

};


/**
 * @brief 
 * 
 * Transition = x.8 - x.2 hier keine Transition in den Testfaellen
 * Versatz = dig=x.n, ana= n.y: kein Versatz, da beide "n" gleich
 */
void test_analogToDigit_Standard() {

    UnderTestCNNGeneral* undertest = new UnderTestCNNGeneral(nullptr, Digital100);

    // 4.8 ist eine "hängende" 5. Heißt sie ist nicht bis auf 5.0 umgesprungen.
    // ab Transition sollte trotzdem ein "hängendes Digit" gerundet werden.
    // Transition = ja
    // Versatz = nein
    TEST_ASSERT_EQUAL_INT(5,  undertest->ZeigerEvalAnalogToDigitNeu(4.8, 8.0, 8, 9.2));

    // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issue-1344032217
    // Standard: dig=9.6, ana=6.8 => erg=9
    // Transition = nein
    // Versatz = nein
    TEST_ASSERT_EQUAL_INT(9,  undertest->ZeigerEvalAnalogToDigitNeu( 9.6, 6.8, 6, 9.2));


    // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1220365920
    // Standard: dig=4.6, ana=6.2 => erg=4
    // Transition = nein
    // Versatz = nein
    TEST_ASSERT_EQUAL_INT(4,  undertest->ZeigerEvalAnalogToDigitNeu( 4.6, 6.2, 6, 9.2));

    // https://github.com/jomjol/AI-on-the-edge-device/issues/1143#issuecomment-1274434805
    // Hängendes digit ()
    // Standard: dig=6.8, ana=8.6 => erg=7
    // Transition = nein
    // Versatz = nein
    TEST_ASSERT_EQUAL_INT(7,  undertest->ZeigerEvalAnalogToDigitNeu( 6.8, 8.6, 6, 9.2));

    // https://github.com/jomjol/AI-on-the-edge-device/issues/1143#issuecomment-1274434805
    // Ebenfalls Hängendes digit () bei kleinem Zeiger nach 0-Durchlauf
    // Standard: dig=6.8, ana=1.0 => erg=7
    // Transition = nein
    // Versatz = nein
    TEST_ASSERT_EQUAL_INT(7,  undertest->ZeigerEvalAnalogToDigitNeu( 6.8, 1.0, 1, 9.2));


}

void test_analogToDigit_Transition() {
    UnderTestCNNGeneral* undertest = new UnderTestCNNGeneral(nullptr, Digital100);
    
    // https://github.com/jomjol/AI-on-the-edge-device/issues/921#issuecomment-1222672175
    // Standard: dig=3.9, ana=9.7 => erg=3
    // Transition = ja
    // Nulldurchgang = nein
    // Versatz = nein
    TEST_ASSERT_EQUAL_INT(3,  undertest->ZeigerEvalAnalogToDigitNeu( 3.9, 9.7, 9, 9.2));
  
    // ohne Referenz
    // Standard: dig=4.0, ana=9.1 => erg=4
    // Transition = ja
    // Nulldurchgang = nein
    // Versatz = nein
    // Besonderheit: Digit ist bei analog 9.1 noch nicht losgelaufen
    TEST_ASSERT_EQUAL_INT(4,  undertest->ZeigerEvalAnalogToDigitNeu( 4.0, 9.1, 9, 9.2));

    // ohne Referenz
    // Standard: dig=9.8, ana=0.1, ana_2=9.9 => erg=9
    // Transition = ja
    // Nulldurchgang = nein
    // Versatz = nein
    // Besonderheit: analog wird durch vorherigen analog wieder auf 9 gesetzt
    TEST_ASSERT_EQUAL_INT(9,  undertest->ZeigerEvalAnalogToDigitNeu( 9.8, 0.1, 9, 9.2));


    // https://github.com/jomjol/AI-on-the-edge-device/issues/1110#issuecomment-1277425333
    // Standard: dig=5.9, ana=9.4 => erg=9
    // Transition = ja
    // Nulldurchgang = nein
    // Versatz = nein
    // Besonderheit: 
    TEST_ASSERT_EQUAL_INT(5,  undertest->ZeigerEvalAnalogToDigitNeu( 5.9, 9.4, 9, 9.2));

}
