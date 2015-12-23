/**
*   Auther : ggg
*   Create Date : 2015-12-23-wed
*   Modified Date : 2015-12-23-wed
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcm_volume_ctl.h"


#define PCM_BYTE_UNIT_8  1
#define PCM_BYTE_UNIT_16 2

#define PCM_SAVE_DATA8(dp, sp)  (sp)[0] = ((BYTE*)(dp))[0];
#define PCM_SAVE_DATA16(dp, sp) (sp)[0] = ((BYTE*)(dp))[0]; sp[1] = ((BYTE*)(dp))[1];
#define PCM_LOAD_DATA8(p)  (SBYTE)(((BYTE*)(p))[0]);
#define PCM_LOAD_DATA16(p) (SWORD)((((BYTE*)(p))[1] << 8) + ((BYTE*)(p))[0]);

#define PCM_MAX_8_BIT  0x7F
#define PCM_MAX_16_BIT 0x7FFF
#define PCM_MIN_8_BIT  -0x80
#define PCM_MIN_16_BIT -0x8000

#define PCM_UP_UNIT 1.25
#define PCM_UP_VOL(d) (d) = (SDWORD)(PCM_UP_UNIT * (d));
#define PCM_DW_UNIT 0.75
#define PCM_DW_VOL(d) (d) = (SDWORD)(PCM_DW_UNIT * (d));


#define PCM_COPY_NATURE(src, dst_dev)               \
    (dst_dev).sample_bits = (src).sample_rate;      \
    (dst_dev).sample_rate = (src).sample_bits;      \
    (dst_dev).channel_count = (src).channel_count;  \
    
#define PCM_SET_NATURE(dst_dev, rate, bits, count)  \
    (dst_dev).sample_bits = (rate);                 \
    (dst_dev).sample_rate = (bits);                 \
    (dst_dev).channel_count = (count);              \
    
#define PCM_CACHE_ORIENT(dst_dev, attr, size)       \
    (dst_dev).indicator = (attr);                   \
    (dst_dev).frame_size = (size);                  \
    

/**
    Cache struct for tample save & process date
*/
typedef struct {
    DWORD frame_size;
    BYTE* indicator;

}pcm_cache;

/**
    Pcm nature struct
*/
typedef struct {
    DWORD sample_rate;
    DWORD sample_bits;
    DWORD channel_count;

}pcm_nature;

/**
    Pcm device struct
*/
typedef struct {
    const dev_st* pdevice_st;
    union {
        const pcm_io_func* pcm_read;
        const pcm_io_func* pcm_write; 
    };
    pcm_nature property;
    pcm_cache cache;
    
}pcm_virtual_device;

/**
    Global device struct
*/
static pcm_virtual_device src_dev, dst_dev;

/**
    Algorithm handle
*/
typedef void pcm_proc_func (const pcm_cache* const cache, DWORD repeat);

/**
    Raise volume for 16 sample bits
*/
static void raise_volume16 (const pcm_cache* const cache, DWORD repeat)
{
    DWORD loop1, loop2;
    SDWORD tmp_value;
    BYTE* indicator = cache->indicator;
    
    for (loop1 = 0; loop1 < cache->frame_size - 1; loop1 += PCM_BYTE_UNIT_16) {
        tmp_value = PCM_LOAD_DATA16(indicator)
        
        for (loop2 = 0; loop2 < repeat; loop2++) {
    		PCM_UP_VOL(tmp_value)
        }
        if (tmp_value > PCM_MAX_16_BIT) {
            tmp_value = PCM_MAX_16_BIT;
        }
        else if (tmp_value < PCM_MIN_16_BIT) {
            tmp_value = PCM_MIN_16_BIT;
        }
        PCM_SAVE_DATA16(&tmp_value, indicator)
        indicator += PCM_BYTE_UNIT_16;
}   }

/**
    Reduce volume for 16 sample bits
*/
static void reduce_volume16 (const pcm_cache* const cache, DWORD repeat)
{
    DWORD loop1, loop2;
    SDWORD tmp_value;
    BYTE* indicator = cache->indicator;
    
    for (loop1 = 0; loop1 < cache->frame_size - 1; loop1 += PCM_BYTE_UNIT_16) {
        tmp_value = PCM_LOAD_DATA16(indicator)
        
        for (loop2 = 0; loop2 < repeat; loop2++) {
    		PCM_DW_VOL(tmp_value)
        }
        PCM_SAVE_DATA16(&tmp_value, indicator)
        indicator += PCM_BYTE_UNIT_16;
}   }

/**
    Raise volume for 8 sample bits
*/
static void raise_volume8 (const pcm_cache* const cache, DWORD repeat)
{
    DWORD loop1, loop2;
    SDWORD tmp_value;
    BYTE* indicator = cache->indicator;
    
    for (loop1 = 0; loop1 < cache->frame_size; loop1 += PCM_BYTE_UNIT_8) {
        tmp_value = PCM_LOAD_DATA8(indicator)
        
        for (loop2 = 0; loop2 < repeat; loop2++) {
    		PCM_UP_VOL(tmp_value)
        }
        if (tmp_value > PCM_MAX_8_BIT) {
            tmp_value = PCM_MAX_8_BIT;
        }
        else if (tmp_value < PCM_MIN_8_BIT) {
            tmp_value = PCM_MIN_8_BIT;
        }
        PCM_SAVE_DATA8(&tmp_value, indicator)
        indicator += PCM_BYTE_UNIT_8;
}   }

/**
    Reduce volume for 8 sample bits
*/
static void reduce_volume8 (const pcm_cache* const cache, DWORD repeat)
{
    DWORD loop1, loop2;
    SDWORD tmp_value;
    BYTE* indicator = cache->indicator;
    
    for (loop1 = 0; loop1 < cache->frame_size ; loop1 += PCM_BYTE_UNIT_8) {
        tmp_value = PCM_LOAD_DATA8(indicator)
        
        for (loop2 = 0; loop2 < repeat; loop2++) {
    		PCM_DW_VOL(tmp_value)
        }
        PCM_SAVE_DATA8(&tmp_value, indicator)
        indicator += PCM_BYTE_UNIT_8;
}   }

/**
    Switch algorithm
*/
static DWORD pcm_volume_regulator (const pcm_cache* const cache, DWORD vol, DWORD sample_bits)
{
    BYTE mode_mask = 0x00;
    if (vol < 0) {
        mode_mask |= 0x01;
        vol = -vol;
    }
    if (16 == sample_bits) mode_mask |= 0x02;
    
    switch (mode_mask){

        case 0x00:  //Up for 8 bits sample
            raise_volume8(cache, vol);
            break;
            
        case 0x02:  //Up for 16 bits sample
            raise_volume16(cache, vol);
            break;
            
        case 0x01:  //Down for 8 bits sample
            reduce_volume8(cache, vol);
            break;
            
        case 0x03:  //Down for 16 bits sample
            reduce_volume16(cache, vol);
            break;
            
        default:
            break;
    }
    return 0;
}

/**
    Get algorithm
*/
static pcm_proc_func* pcm_get_regulator (const pcm_cache* const cache, DWORD vol, DWORD sample_bits)
{
    pcm_proc_func* ret = NULL;
    
    BYTE mode_mask = 0x00;
    if (vol < 0) {
        mode_mask |= 0x01;
        vol = -vol;
    }
    if (16 == sample_bits) mode_mask |= 0x02;
    
    switch (mode_mask){

        case 0x00:  //Up for 8 bits sample
            ret = raise_volume8;
            break;
            
        case 0x02:  //Up for 16 bits sample
            ret = raise_volume16;
            break;
            
        case 0x01:  //Down for 8 bits sample
            ret = reduce_volume8;
            break;
            
        case 0x03:  //Down for 16 bits sample
            ret = reduce_volume16;
            break;
            
        default:
            break;
    }
    return ret;
}

/**
    process loop
*/
DWORD pcm_vol_process_loop (DWORD vol)
{
    while (1) {
        dst_dev.cache.frame_size = src_dev.pcm_read(src_dev.pdevice_st, src_dev.cache.indicator, src_dev.cache.frame_size);
        if (0 == dst_dev.cache.frame_size) {
            break;
        }
        pcm_volume_regulator(&(dst_dev.cache), vol, dst_dev.property.sample_bits);
        
        dst_dev.cache.frame_size = dst_dev.pcm_write(dst_dev.pdevice_st, dst_dev.cache.indicator, dst_dev.cache.frame_size);
        if (0 == dst_dev.cache.frame_size) {
            break;
    }   }
    return 0;
}


/**
    Simple optimize for process pcm files or memory buffer
*/
DWORD pcm_vol_process_loop_obtuse (DWORD vol)
{
    pcm_proc_func* proc_func = pcm_get_regulator(&(dst_dev.cache), vol, dst_dev.property.sample_bits);
    if (vol < 0) vol = -vol;
    
    while (1) {
        dst_dev.cache.frame_size = src_dev.pcm_read(src_dev.pdevice_st, src_dev.cache.indicator, src_dev.cache.frame_size);
        if (0 == dst_dev.cache.frame_size) {
            break;
        }
        proc_func(&(dst_dev.cache), vol);
        
        dst_dev.cache.frame_size = dst_dev.pcm_write(dst_dev.pdevice_st, dst_dev.cache.indicator, dst_dev.cache.frame_size);
        if (0 == dst_dev.cache.frame_size) {
            break;
    }   }
    return 0;
}

/**
    provide a buffer which is created by your own alloc func
*/
DWORD pcm_cache_set (BYTE* buffer, DWORD length)
{
    PCM_CACHE_ORIENT(src_dev.cache, buffer, length)
    PCM_CACHE_ORIENT(dst_dev.cache, src_dev.cache.indicator, src_dev.cache.frame_size)
    
    return 0;
}

/**
    Set pcm nature, sample rate, sample bits, channel count
    only sample bits is valid 
*/
DWORD pcm_nature_set (DWORD sample_rate, DWORD sample_bits, DWORD channel_count)
{
    PCM_SET_NATURE(src_dev.property, sample_rate, sample_bits, channel_count)
    PCM_COPY_NATURE(src_dev.property, dst_dev.property)
    
    return 0;
}

DWORD pcm_device_init (const dev_st* const src, const dev_st* const dst,
                        const pcm_io_func* const pcm_read, const pcm_io_func* const pcm_write)
{
    src_dev.pdevice_st = src;
    src_dev.pcm_read = pcm_read;
    
    dst_dev.pdevice_st = dst;
    dst_dev.pcm_write = pcm_write;
    
    return 0;
}

/**
    For memory buffer operate
*/
DWORD pcm_dummy_read (const void* const device_st, BYTE* const buffer, DWORD length)
{
    memcpy(buffer, (BYTE*)device_st, length);

    return 0;
}

DWORD pcm_dummy_write (const void* const device_st, BYTE* const buffer, DWORD length)
{
    memcpy((BYTE*)device_st, buffer, length);

    return 0;
}

/**
    For file & linux device operate
*/
DWORD pcm_read (const void* const device_st, BYTE* const buffer, DWORD length)
{
    return fread(buffer, 1, length, (FILE*)device_st);
}

DWORD pcm_write (const void* const device_st, BYTE* const buffer, DWORD length)
{
    return fwrite(buffer, 1, length, (FILE*)device_st);
}

int main (int argc, char* argv[])
{
    FILE* fpr = fopen(argv[1], "rb");
    FILE* fpw = fopen(argv[2], "wb");

    BYTE buf_cache[10] = {0};
    
    pcm_device_init(fpr, fpw, (const pcm_io_func*)&pcm_read, (const pcm_io_func*)&pcm_write);
    
    pcm_nature_set (1, argv[3], 1);
    
    pcm_cache_set (buf_cache, 10);
    pcm_vol_process_loop_obtuse(atoi(argv[4]));

    fclose(fpr);
    fclose(fpw);

	return 0;
}
