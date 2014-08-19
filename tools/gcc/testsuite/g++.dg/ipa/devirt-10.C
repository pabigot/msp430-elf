/* { dg-do compile } */
/* { dg-options "-O3 -fdump-ipa-inline -fdump-ipa-cp -fno-early-inlining" } */
class wxPaintEvent {  };
struct wxDCBase
{ 
  wxDCBase ();
  virtual int GetLayoutDirection() const{}
  virtual void SetLayoutDirection(int){}
};
struct wxWindowDC  : public wxDCBase {};
struct wxBufferedDC  : public wxDCBase
{ 
  void Init(wxDCBase*dc) {
    InitCommon(dc);
  }
  void InitCommon(wxDCBase*dc) {
    if (dc)
      SetLayoutDirection(dc->GetLayoutDirection());
  }
};
struct wxBufferedPaintDC  : public wxBufferedDC {
  wxBufferedPaintDC() {
    Init(&m_paintdc);
  }
 wxWindowDC m_paintdc;
};
void  OnPaint(wxPaintEvent & event) {
  wxBufferedPaintDC dc;
}
/* IPA-CP should really discover both cases, but for time being the second is handled by inliner.  */
/* { dg-final { cleanup-ipa-dump "inline" } } */
/* { dg-final { cleanup-ipa-dump "cp" } } */
