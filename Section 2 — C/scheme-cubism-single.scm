(define (simple-cubism
        filename
        tile-size
        saturation
        bg-color)
    (let* ((image (car (gimp-file-load RUN-NONINTERACTIVE filename filename)))
        (drawable (car (gimp-image-get-active-layer image))))
    (plug-in-cubism RUN-NONINTERACTIVE
        image drawable tile-size saturation bg-color)
    (gimp-file-save RUN-NONINTERACTIVE image drawable filename filename)
    (gimp-image-delete image)))
