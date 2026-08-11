/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 8 (PPLIB).
 * Web: https://github.com/pfedick/pplib
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#ifndef PPLIB_INCLUDE_GRAFIX_IMAGEFILTER_H
#define PPLIB_INCLUDE_GRAFIX_IMAGEFILTER_H

#include <stdint.h>

#include <pplib/types/string.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/assocarray.h>
#include <pplib/core/fileobject.h>
#include <pplib/core/baseexception.h>
#include <pplib/grafix/drawable.h>

#include <pplib/grafix/rgbformat.h>

namespace pplib::grafix
{

typedef struct IMAGE
{
    int width;
    int height;
    int pitch;
    int bitdepth;
    int colors;
    RGBFormat format;
} IMAGE;

/** @class ImageFilter
 * @ingroup PPLGroupGrafik
 * @brief Basisklasse für Import-/Export-Filter verschiedener Grafikformate
 *
 * Dies ist die Abstrakte Basisklasse für alle Import- und Export-Filter für verschiedene
 * Grafikformate. Eine Instanz des Filters muss mit "new" angelegt und mit der Funktion
 * Grafix::addFilter in der Grafik-Engine registriert werden. Die Grafik-Engine
 * kümmert sich bei Programmende selbst um das Löschen des Filters.
 * Mit Grafix::unloadFilter kann ein Filter aber auch manuell von der Anwendung
 * wieder entfernt werden. Mit Grafix::findFilter kann ein Filter anhand seines
 * Namens gefunden werden.
 */
class ImageFilter
{
private:
public:
    PPLIBEXCEPTION(IllegalImageFormatException, Exception);
    PPLIBEXCEPTION(EmptyImageException, Exception);

    /** @brief Destruktor
     *
     * Der Destruktor sorgt dafür, dass durch die Klasse allokierter Speicher
     * wieder freigegeben wird.
     */
    virtual ~ImageFilter() = default;

    /** @brief Grafikformat identifizieren
     *
     * Diese Funktion wird aufgerufen, um herauszufinden, ob ein Grafikformat durch diesen
     * Filter gelesen werden kann. Ist dieser der Fall, gibt die Funktion True zurück und
     * füllt die IMAGE-Struktur \p img mit den Informationen zum Bild. Wird das Format nicht
     * unterstützt, wird False zurückgegeben.
     *
     * @param[in] file Eine geöffnete Datei
     * @param[out] img Referenz auf eine IMAGE-Struktur
     * @return True, wenn das Format unterstützt wird, ansonsten False.
     */
    virtual bool ident(FileObject& file, IMAGE& img) noexcept = 0;

    /** @brief Grafik in ein Drawable laden
     *
     * Diese Funktion wird aufgerufen, um ein Bild aus einer Datei zu laden. Die Funktion
     * füllt die Drawable-Struktur \p surface mit den Bilddaten. Die IMAGE-Struktur \p img
     * enthält die Informationen zum Bild.
     *
     * @param[in] file Eine geöffnete Datei
     * @param[out] surface Referenz auf eine Drawable-Struktur
     * @param[in] img Referenz auf eine IMAGE-Struktur
     */
    virtual void load(FileObject& file, Drawable& surface, IMAGE& img) = 0;
};

class ImageFilter_PNG : public ImageFilter
{
public:
    enum class ColorType
    {
        Auto = 0, ///< Automatisch anhand des Drawable-Formats wählen
        RGB,      ///< 24 Bit TrueColor (RGB)
        RGBA,     ///< 32 Bit TrueColor mit Alpha (RGBA)
        Gray,     ///< 8 Bit Graustufe
        GrayAlpha ///< 16 Bit Graustufe mit Alpha
    };

    enum class Compression
    {
        Default = -1, ///< Zlib-Standard (Level 6)
        None = 0,     ///< Keine Kompression (schnellstmöglich)
        Fast = 1,     ///< Schnelle Kompression
        Best = 9      ///< Maximale Kompression (kleinste Datei)
    };

    ImageFilter_PNG();
    ~ImageFilter_PNG() override;
    bool ident(FileObject& file, IMAGE& img) noexcept override;
    void load(FileObject& file, Drawable& surface, IMAGE& img) override;

    /** @brief Grafik in eine Datei speichern
     *
     * Diese Funktion wird aufgerufen, um ein Bild in eine Datei zu speichern. Die Funktion
     * liest die Bilddaten aus der Drawable-Struktur \p surface und speichert sie in der
     * Datei \p file. Die Parameter \p param können zusätzliche Informationen zum Speichern
     * enthalten, z.B. die Qualität bei JPEG-Bildern.
     *
     * @param[in] surface Referenz auf eine Drawable-Struktur
     * @param[in] file Eine geöffnete Datei
     * @param[in] param Referenz auf eine AssocArray-Struktur mit zusätzlichen Parametern
     */
    void save(const Drawable& surface,
              FileObject& file,
              ColorType color_type = ColorType::Auto,
              Compression compression = Compression::Default) const;

    /** @brief Grafik in eine Datei speichern
     *
     * Diese Funktion wird aufgerufen, um ein Bild in eine Datei zu speichern. Die Funktion
     * liest die Bilddaten aus der Drawable-Struktur \p surface und speichert sie in der
     * Datei \p file. Die Parameter \p param können zusätzliche Informationen zum Speichern
     * enthalten, z.B. die Qualität bei JPEG-Bildern.
     *
     * @param[in] surface Referenz auf eine Drawable-Struktur
     * @param[in] filename Name der Zieldatei
     * @param[in] param Referenz auf eine AssocArray-Struktur mit zusätzlichen Parametern
     */
    void saveFile(const String& filename,
                  const Drawable& surface,
                  ColorType color_type = ColorType::Auto,
                  Compression compression = Compression::Default) const;
};

class ImageFilter_JPEG : public ImageFilter
{
public:
    enum class DctMethod : uint8_t
    {
        SlowAccurate = 0, ///< JDCT_ISLOW
        FastInt = 1,      ///< JDCT_IFAST, legacy integer method
        Float = 2         ///< JDCT_FLOAT
    };

    class Options
    {
    public:
        uint8_t quality = 85; ///< 0-100
        uint8_t smooth = 0;   ///< 0-100
        DctMethod dct_method = DctMethod::SlowAccurate;
        bool force_baseline = true;
        bool optimized = false;

        Options() {};
    };

    ImageFilter_JPEG();
    ~ImageFilter_JPEG() override;
    bool ident(FileObject& file, IMAGE& img) noexcept override;
    void load(FileObject& file, Drawable& surface, IMAGE& img) override;
    void save(const Drawable& surface, FileObject& file, const Options& options = Options()) const;
    void saveFile(const String& filename, const Drawable& surface, const Options& options = Options()) const;
};

class ImageFilter_BMP : public ImageFilter
{
public:
    ImageFilter_BMP();
    ~ImageFilter_BMP() override;
    bool ident(FileObject& file, IMAGE& img) noexcept override;
    void load(FileObject& file, Drawable& surface, IMAGE& img) override;
    void save(const Drawable& surface, FileObject& file) const;
    void saveFile(const String& filename, const Drawable& surface) const;
};

class ImageFilter_GIF : public ImageFilter
{
public:
    ImageFilter_GIF();
    ~ImageFilter_GIF() override;
    bool ident(FileObject& file, IMAGE& img) noexcept override;
    void load(FileObject& file, Drawable& surface, IMAGE& img) override;
    // Keine save-Funktion, da GIF nur 256 Farben unterstützt und das Drawable das noch nicht unterstützt.
};

class ImageFilter_PPM : public ImageFilter
{
public:
    ImageFilter_PPM();
    ~ImageFilter_PPM() override;
    bool ident(FileObject& file, IMAGE& img) noexcept override;
    void load(FileObject& file, Drawable& surface, IMAGE& img) override;
    void saveFile(const String& filename, const Drawable& surface, bool asAscii = false) const;
    void save(const Drawable& surface, FileObject& file, bool asAscii = false) const;
};

class ImageFilter_TGA : public ImageFilter
{
public:
    ImageFilter_TGA();
    ~ImageFilter_TGA() override;
    bool ident(FileObject& file, IMAGE& img) noexcept override;
    void load(FileObject& file, Drawable& surface, IMAGE& img) override;
};

class ImageFilter_TIFF : public ImageFilter
{
public:
    ImageFilter_TIFF();
    ~ImageFilter_TIFF() override;
    bool ident(FileObject& file, IMAGE& img) noexcept override;
    void load(FileObject& file, Drawable& surface, IMAGE& img) override;
};

} // namespace pplib::grafix

#endif // PPLIB_INCLUDE_GRAFIX_IMAGEFILTER_H
