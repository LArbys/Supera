#include "FilterEmptyImages.h"
#include <iostream>
#include "SuperaCore/caffe.pb.h"
#include "TF1.h"
#include "TMath.h"

namespace larcaffe {

  namespace supera {

    FilterEmptyImages::FilterEmptyImages() {
      // is this going cuase weird problems somewhere?
      hadcs = new TH1D("adc_filter_empty_images", "", 4096, 0, 4096);
    }

    FilterEmptyImages::~FilterEmptyImages() {
      delete hadcs;
    }

    void FilterEmptyImages::configure( fhicl::ParameterSet const & p ) {
      sigmaToCheck = p.get<int>("SigmaToCheck",3);
      ratioThreshold = p.get<float>("RatioThreshold",1.0);
    }

    bool FilterEmptyImages::doWeKeep( const ::larcaffe::supera::converter_base& img_data  ) { 
      //std::cout << "[FilterEmptyImages] Test. Do Nothing." << std::endl;
      // This is a simple filter algorithm.
      // We histogram the ADC values and then fit a gaussian to the maximum
      // Then we cut based on how many adc values are above a certain threshold
      
      // Reset Histogram
      hadcs->Reset();

      // Image Data 
      const ::caffe::Datum& datum = img_data.data();
      
      // Fill ADC histogram
      for ( int h=0; h<datum.height(); h++ ) {
	for ( int w=0; w<datum.width(); w++ ) {
	  float adc = datum.float_data( h*datum.width() + w );
	  hadcs->Fill( adc );
	}
      }

      int maxbin = hadcs->GetMaximumBin();
      double maxadc = hadcs->GetBinLowEdge( maxbin );
      TF1* f = new TF1("adcfit", "gaus(0)", maxadc-20, maxadc+20);
      f->SetParameter( 0, hadcs->GetMaximum() );
      f->SetParameter( 1, hadcs->GetMean() );
      f->SetParameter( 2, hadcs->GetRMS() );
      hadcs->Fit( f, "RQ", "" );
      // for (int b=1; b<=hadcs->GetXaxis()->GetNbins(); b++ ) {
      // 	std::cout << "[bin " << b << "] " << hadcs->GetBinContent( b ) << std::endl;
      // }

      int fitted_mean_adc  = hadcs->FindBin( f->GetParameter(1) );
      int fitted_sigma_adc = f->GetParameter(2)+1;
      int hightail_tot = 0;
      for (int b=fitted_mean_adc+(int)(sigmaToCheck*fitted_sigma_adc); b<hadcs->GetXaxis()->GetNbins(); b++ ) {
	hightail_tot +=  (int)hadcs->GetBinContent(b+1);
      }
      //std::cout << "Image H:" << datum.height() << " W:" << datum.width() << " Pixels:" << datum.height()*datum.width() << std::endl;
      //std::cout << "HADC mean=" << hadcs->GetMean() << " rms=" << hadcs->GetRMS() << " maxadc=" << maxadc << std::endl;
      
      // CDF
      double cdf = 0.5*(1 + TMath::Erf( ((double)(fitted_mean_adc+fitted_sigma_adc*sigmaToCheck) - f->GetParameter(1))/(TMath::Sqrt(2)*f->GetParameter(2)) ) );
      //std::cout << "CDF(@" << fitted_sigma_adc << "adcs=sigma): " << 1.0-cdf << std::endl;

      // Integral
      double tail_integral = ((double)hightail_tot)/hadcs->Integral();
      double tail_ratio = tail_integral/(1.0-cdf);
      //std::cout << "Tail Integral: " << tail_integral << " = " << hightail_tot << "/" << hadcs->Integral() << std::endl;
      //std::cout << "Tail Ratio: " << tail_ratio << std::endl;
      if ( tail_ratio < ratioThreshold ) {
	std::cout << "[FilterEmptyImages] Empty! ratio(" << tail_ratio << ") < threshold(" << ratioThreshold << ")" << std::endl;
	return false;
      }
      //std::cout << "[FilterEmptyImages] Tail Bigger than expectation: " << tail_ratio << ">" << ratioThreshold << std::endl;

      delete f;
      
      return true; 
    }
    
    // This creates a global instance of the Concrete Factory, which will self-register into the Factory Map in FilterBase
    static FilterEmptyImagesFactory global_FilterEmptyImagesFactory;

  }

}
