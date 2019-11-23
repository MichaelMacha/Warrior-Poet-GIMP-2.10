 (define (batch-cubism
    pattern
    tile-size
    saturation
    bg-color)
(let* ((filelist (cadr (file-glob pattern 1))))
    (while (not (null? filelist))
        (let* ((filename (car filelist))
            (image (car (gimp-file-load RUN-NONINTERACTIVE
                filename filename)))
                    (drawable (car (gimp-image-get-active-layer image))))
            (plug-in-cubism RUN-NONINTERACTIVE
                image drawable tile-size saturation bg-color)
            (gimp-file-save RUN-NONINTERACTIVE
                image drawable filename filename)
                (gimp-image-delete image))
            (set! filelist (cdr filelist)))))
