#ifndef PCM_VOLUME_CTL_H
#define PCM_VOLUME_CTL_H


#ifndef DWORD
#define DWORD unsigned int
#endif
#ifndef SDWORD
#define SDWORD int
#endif
#ifndef SWORD
#define SWORD short
#endif
#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef SBYTE
#define SBYTE char
#endif

typedef void dev_st;
typedef DWORD pcm_io_func (const dev_st* const device_st, BYTE* const buffer, DWORD length);

extern DWORD pcm_cache_set (BYTE* const buffer, DWORD length);

extern DWORD pcm_device_init (const dev_st* const src, const dev_st* const dst,
                                const pcm_io_func* const pcm_read, const pcm_io_func* const pcm_write);

extern DWORD pcm_nature_set (DWORD sample_rate, DWORD sample_bits, DWORD channel_count);

extern DWORD pcm_vol_process_loop (DWORD vol);

extern DWORD pcm_vol_process_loop_obtuse (DWORD vol);

#endif
