/**
 * @private
 */
class OneSampleDelay {
public:

  float in = 0;
  float prevIn = 0;
  float out = 0;

  OneSampleDelay() {

  }

  void process() {
    out = prevIn;
    prevIn = in;
  }
};
