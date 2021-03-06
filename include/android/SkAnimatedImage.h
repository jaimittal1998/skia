/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnimatedImage_DEFINED
#define SkAnimatedImage_DEFINED

#include "SkBitmap.h"
#include "SkCodecAnimation.h"
#include "SkDrawable.h"
#include "SkMatrix.h"
#include "SkRect.h"

class SkAndroidCodec;
class SkPicture;

/**
 *  Thread unsafe drawable for drawing animated images (e.g. GIF).
 */
class SK_API SkAnimatedImage : public SkDrawable {
public:
    /**
     *  Create an SkAnimatedImage from the SkAndroidCodec.
     *
     *  Returns null on failure to allocate pixels. On success, this will
     *  decode the first frame. It will not animate until start() is called.
     *
     *  @param scaledSize Size to draw the image, possibly requiring scaling.
     *  @param cropRect Rectangle to crop to after scaling.
     *  @param postProcess Picture to apply after scaling and cropping.
     */
    static sk_sp<SkAnimatedImage> Make(std::unique_ptr<SkAndroidCodec>,
            SkISize scaledSize, SkIRect cropRect, sk_sp<SkPicture> postProcess);

    /**
     *  Simpler version that uses the default size, no cropping, and no postProcess.
     */
    static sk_sp<SkAnimatedImage> Make(std::unique_ptr<SkAndroidCodec>);

    ~SkAnimatedImage() override;

    /**
     *  Start or resume the animation. update() must be called to advance the
     *  time.
     */
    void start();

    /**
     *  Stop the animation. update() has no effect while the animation is
     *  stopped.
     */
    void stop();

    /**
     *  Reset the animation to the beginning.
     */
    void reset();

    /**
     *  Whether the animation is active.
     *
     *  If true, update() can be called to animate.
     */
    bool isRunning() const { return fRunning && !fFinished; }

    /**
     *  Whether the animation completed.
     *
     *  Returns true after all repetitions are complete, or an error stops the
     *  animation. Gets reset to false if the animation is restarted.
     */
    bool isFinished() const { return fFinished; }

    /**
     * Returned by update if the animation is not running.
     */
    static constexpr double kNotRunning = -2.0;

    /**
     *  Update the current time. If the image is animating, this may decode
     *  a new frame.
     *
     *  @return the time to show the next frame, or kNotRunning if the animation
     *      is not running.
     */
    double update(double msecs);

    /**
     *  Change the repetition count.
     *
     *  By default, the image will repeat the number of times indicated in the
     *  encoded data.
     *
     *  Use SkCodec::kRepetitionCountInfinite for infinite, and 0 to show all
     *  frames once and then stop.
     */
    void setRepetitionCount(int count);

protected:
    SkRect onGetBounds() override;
    void onDraw(SkCanvas*) override;

private:
    struct Frame {
        SkBitmap fBitmap;
        int      fIndex;
        SkCodecAnimation::DisposalMethod fDisposalMethod;

        Frame();
        bool copyTo(Frame*) const;
    };

    std::unique_ptr<SkAndroidCodec> fCodec;
    const SkISize                   fScaledSize;
    const SkImageInfo               fDecodeInfo;
    const SkIRect                   fCropRect;
    const sk_sp<SkPicture>          fPostProcess;
    const int                       fFrameCount;
    const bool                      fSimple;     // no crop, scale, or postprocess
    SkMatrix                        fMatrix;     // used only if !fSimple

    bool                            fFinished;
    bool                            fRunning;
    double                          fNowMS;
    double                          fRemainingMS;
    Frame                           fActiveFrame;
    Frame                           fRestoreFrame;
    int                             fRepetitionCount;
    int                             fRepetitionsCompleted;

    SkAnimatedImage(std::unique_ptr<SkAndroidCodec>, SkISize scaledSize,
            SkImageInfo decodeInfo, SkIRect cropRect, sk_sp<SkPicture> postProcess);
    SkAnimatedImage(std::unique_ptr<SkAndroidCodec>);

    int computeNextFrame(int current, bool* animationEnded);
    double finish();

    typedef SkDrawable INHERITED;
};

#endif // SkAnimatedImage_DEFINED
