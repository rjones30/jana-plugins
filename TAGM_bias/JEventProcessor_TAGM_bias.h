// $Id$
//
//    File: JEventProcessor_TAGM_bias.h
// Created: Fri Nov 17 09:21:56 EST 2017
// Creator: aebarnes (on Linux ifarm1401.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_TAGM_bias_
#define _JEventProcessor_TAGM_bias_

#include <JANA/JEventProcessor.h>


class JEventProcessor_TAGM_bias : public JEventProcessor {
 public:
   JEventProcessor_TAGM_bias();
  ~JEventProcessor_TAGM_bias();
  const char* className(void){return "JEventProcessor_TAGM_bias";}

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
};

#endif // _JEventProcessor_TAGM_bias_

