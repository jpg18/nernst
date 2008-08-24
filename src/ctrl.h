/* ctrl.h
 *
 */

#ifndef CTRL_H
#define CTRL_H

#include <QWidget>


class QLabel;
class QLineEdit;
class QSlider;
class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class QStackedLayout;


class NernstCtrl : public QWidget
{
   Q_OBJECT

   public:
      NernstCtrl( struct options *o, QWidget *parent = 0 );

   public slots:
      void updateIter( int iter );
      void changeIters( int iters );
      void changeX( int xpow );
      void changeY( int ypow );
      void changeCapacitance( int cap );
      void changeSeed( QString seed );
      void changeLeftK( int lK );
      void changeLeftK( QString lK );
      void changeLeftNa( int lNa );
      void changeLeftNa( QString lNa );
      void changeLeftCl( int lCl );
      void changeLeftCl( QString lCl );
      void changeRightK( int rK );
      void changeRightK( QString rK );
      void changeRightNa( int rNa );
      void changeRightNa( QString rNa );
      void changeRightCl( int rCl );
      void changeRightCl( QString rCl );
      void changePermK( int pK );
      void changePermK( QString pK );
      void changePermNa( int pNa );
      void changePermNa( QString pNa );
      void changePermCl( int pCl );
      void changePermCl( QString pCl );
      void changeSelectivity( bool selectivity );
      void changeElectrostatics( bool electrostatics );
      void reloadSettings();

      void clearTrackedIons();
      void disableCtrl();
      void reenableCtrl();
      void resetCtrl();

   signals:
      void startBtnClicked();
      void pauseBtnClicked();
      void continueBtnClicked();
      void clearTrackingBtnClicked();
      void resetBtnClicked();
      void quitBtnClicked();

      void adjustTable();
      void updatePreview();
 
   private:
      struct options *o;

      int currentIter;
      int itersDefault;
      int xDefault;
      int yDefault;
      double capDefault;
      int lKDefault;
      int lNaDefault;
      int lClDefault;
      int rKDefault;
      int rNaDefault;
      int rClDefault;
      double pKDefault;
      double pNaDefault;
      double pClDefault;
      int selectivityDefault;
      int electrostaticsDefault;

      QLabel *headerLbl;

      QLabel *itersLbl;
      QSlider *itersSld;
      QLabel *itersVal;

      QLabel *xLbl;
      QSlider *xSld;
      QLabel *xVal;

      QLabel *yLbl;
      QSlider *ySld;
      QLabel *yVal;

      QLabel *capLbl;
      QSlider *capSld;
      QLabel *capVal;

      QLabel *seedLbl;
      QLineEdit *seedVal;

      QLabel *inLbl;
      QLabel *outLbl;
      QLabel *permLbl;
      QLabel *KLbl;
      QLabel *NaLbl;
      QLabel *ClLbl;
      QLabel *mMLbl1;
      QLabel *mMLbl2;
      QLabel *mMLbl3;
      QLabel *mMLbl4;
      QLabel *mMLbl5;
      QLabel *mMLbl6;
      QSlider *lKSld;
      QLineEdit *lKVal;
      QSlider *lNaSld;
      QLineEdit *lNaVal;
      QSlider *lClSld;
      QLineEdit *lClVal;
      QSlider *rKSld;
      QLineEdit *rKVal;
      QSlider *rNaSld;
      QLineEdit *rNaVal;
      QSlider *rClSld;
      QLineEdit *rClVal;
      QSlider *pKSld;
      QLineEdit *pKVal;
      QSlider *pNaSld;
      QLineEdit *pNaVal;
      QSlider *pClSld;
      QLineEdit *pClVal;

      QCheckBox *selectivity;
      QCheckBox *electrostatics;

      QPushButton *startBtn;
      QPushButton *pauseBtn;
      QPushButton *continueBtn;
      QPushButton *clearTrackingBtn;
      QPushButton *resetBtn;
      QPushButton *quitBtn;

      QVBoxLayout *mainLayout;
      QGridLayout *ctrlLayout;
      QGroupBox *sldBox;
      QGridLayout *sldLayout;
      QStackedLayout *stackedBtnLayout;
};

#endif /* CTRL_H */
