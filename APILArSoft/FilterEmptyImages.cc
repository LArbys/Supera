#include "FilterEmptyImages.h"
#include <iostream>
#include "SuperaCore/caffe.pb.h"
#include "TF1.h"
#include "TMath.h"
#include "TFile.h"

namespace larcaffe {

  namespace supera {

    FilterEmptyImages::FilterEmptyImages() {
      // is this going cuase weird problems somewhere?
      hadcs = new TH1D("adc_filter_empty_images", "", 4096*5, 0, 4096);
    }

    FilterEmptyImages::~FilterEmptyImages() {
      delete hadcs;
    }

    void FilterEmptyImages::configure( fhicl::ParameterSet const & p ) {
      sigmaToCheck = p.get<int>("SigmaToCheck",3);
      ratioThreshold = p.get<float>("RatioThreshold",1.0);
      verbosity = p.get<int>("Verbosity",0);
    }

    bool FilterEmptyImages::doWeKeep( const ::larcaffe::supera::converter_base& img_data ) { 
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

      // TFile* temp = new TFile("temp.root", "RECREATE");
      // temp->cd();
      // hadcs->Write("hadcs");
      // temp->Close();

      int maxbin = hadcs->GetMaximumBin();
      double maxadc = hadcs->GetBinLowEdge( maxbin );
      TF1* f = new TF1("adcfit", "gaus(0)", maxadc-20, maxadc+20);
      f->SetParameter( 0, hadcs->GetMaximum() );
      f->SetParameter( 1, hadcs->GetMean() );
      f->SetParameter( 2, hadcs->GetRMS() );
      hadcs->Fit( f, "RQ", "", hadcs->GetMean()-2*hadcs->GetRMS(), hadcs->GetMean()+2*hadcs->GetRMS() );
      // for (int b=1; b<=hadcs->GetXaxis()->GetNbins(); b++ ) {
      // 	std::cout << "[bin " << b << "] " << hadcs->GetBinContent( b ) << std::endl;
      // }

      float fitted_mean_adc  = f->GetParameter(1);
      float fitted_sigma_adc = f->GetParameter(2);
      //int fitted_mean_bin = hadcs->FindBin( fitted_mean_adc );
      int tail_start_bin = hadcs->FindBin( fitted_mean_adc+sigmaToCheck*fitted_sigma_adc )+1;
      float start_bin_val = hadcs->GetXaxis()->GetBinLowEdge( tail_start_bin );
      float tail_integral = 0;
      for (int b=tail_start_bin; b<=hadcs->GetXaxis()->GetNbins(); b++ ) {
	tail_integral +=  hadcs->GetBinContent(b);
      }
      //std::cout << "Image H:" << datum.height() << " W:" << datum.width() << " Pixels:" << datum.height()*datum.width() << std::endl;
      if ( verbosity )
	std::cout << "[FilterEmptyImage] HADC mean=" << hadcs->GetMean() << " rms=" << hadcs->GetRMS() << " maxadc=" << maxadc << std::endl;
      
      // CDF
      double cdf = 0.5*(1 + TMath::Erf( (start_bin_val - fitted_mean_adc)/(TMath::Sqrt(2)*fitted_sigma_adc) ) );
      if ( verbosity )
	std::cout << "[FilterEmptyImage] CDF(@" << fitted_sigma_adc << "adcs=sigma): " << 1.0-cdf << std::endl;
      
      // Integral
      double tail_expectation = hadcs->Integral()*(1.0-cdf);
      if ( verbosity ) 
	std::cout << "[FilterEmptyImages] "
		  << " Tail Integral: " << tail_integral << " vs. expectation " << tail_expectation
		  << " Tail Ratio=" << tail_integral/tail_expectation
		  << std::endl;
      
      if ( tail_integral < tail_expectation+sqrt(tail_expectation)*3.0 ) {
	std::cout << "[FilterEmptyImages] Empty! tail_integral(" << tail_integral << ") < tail_expectation+3sig (" << tail_expectation+sqrt(tail_expectation)*3.0 << ")" << std::endl;
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
