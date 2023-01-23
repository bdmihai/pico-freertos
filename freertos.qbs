/*_____________________________________________________________________________
 │                                                                            |
 │ COPYRIGHT (C) 2023 Mihai Baneu                                             |
 │                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 23.Jan.2023  |
 |                                                                            |
 |___________________________________________________________________________*/

import qbs.FileInfo

Product {
    name: 'freertos'
    type: 'lib'

    Depends { name: 'rp' }
    Depends { name: 'cmsis' }
    Depends { name: 'rp2040' }
    Depends { name: 'pico_util' }
    Depends { name: 'pico_time' }
    Depends { name: 'pico_sync' }
    Depends { name: 'pico_multicore' }
    Depends { name: 'hardware_timer' }
    Depends { name: 'hardware_sync' }

    rp.includePaths: [ 'inc', 'port' ]
    rp.defines: [ 
        'FREERTOS_IN_RAM=0'
    ]

    files: [
        'inc/*.h',
        'src/*.c',
        'port/*.c',
        'port/*.h',
        'port/*.S',
    ]

    Export {
        Depends { name: 'rp' }
        Depends { name: 'cmsis' }
        Depends { name: 'rp2040' }
        Depends { name: 'pico_util' }
        Depends { name: 'pico_time' }
        Depends { name: 'pico_sync' }
        Depends { name: 'pico_multicore' }
        Depends { name: 'hardware_timer' }
        Depends { name: 'hardware_sync' }
        
        rp.includePaths: [
            FileInfo.joinPaths(exportingProduct.sourceDirectory, '/inc'),
            FileInfo.joinPaths(exportingProduct.sourceDirectory, '/port')
        ]
        rp.libraryPaths: [ exportingProduct.destinationDirectory ]
        rp.linkerFlags: ['-Wl,--undefined=uxTopUsedPriority']
    }
}
