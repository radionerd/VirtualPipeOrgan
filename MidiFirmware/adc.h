#ifndef __ADC_H
#define __ADC_H


#ifdef __cplusplus
 extern "C" {
#endif


class ADC {
  private:
  public:
  void Begin(void);
  void Scan ( void );
  void Print ( void );
};

#ifdef __cplusplus
}
#endif 


#endif /* __ADC_H */
