#include <stdio.h>
#include <stdint.h>
#define _ATTRIBUTE_PACKED_  __attribute__((packed, aligned(1)))


typedef struct {
  float x;
  float y;
  float z;
} _ATTRIBUTE_PACKED_  vector31_t;


typedef struct {
  float x;
  float y;

} _ATTRIBUTE_PACKED_  vector21_t;


typedef struct {
  /*00*/ char  signature[4];
  /*04*/ uint8_t  versions;
  /*05*/ uint8_t  video_codec;
  /*06*/ uint16_t header_size;
  /*08*/ uint32_t payload_size;             /* Amount of data following this PaVE */
  /*12*/ uint16_t encoded_stream_width;     /* ex: 640 */
  /*14*/ uint16_t encoded_stream_height;    /* ex: 368 */
  /*16*/ uint16_t display_width;            /* ex: 640 */
  /*18*/ uint16_t display_height;           /* ex: 360 */
  /*20*/ uint32_t frame_number;             /* frame position inside the current stream */
  /*24*/ uint32_t timestamp;                /* in milliseconds */
  /*28*/ uint8_t  total_chuncks;            /* number of UDP packets containing the current decodable payload */
  /*29*/ uint8_t  chunck_index ;            /* position of the packet - first chunk is #0 */
  /*30*/ uint8_t  frame_type;               /* I-frame, P-frame */
  /*31*/ uint8_t  control;                  /* Special commands like end-of-stream or advertised frames */
  /*32*/ uint32_t stream_byte_position_lw;  /* Byte position of the current payload in the encoded stream  - lower 32-bit word */
  /*36*/ uint32_t stream_byte_position_uw;  /* Byte position of the current payload in the encoded stream  - upper 32-bit word */
  /*40*/ uint16_t stream_id;                /* This ID indentifies packets that should be recorded together */
  /*42*/ uint8_t  total_slices;             /* number of slices composing the current frame */
  /*43*/ uint8_t  slice_index ;             /* position of the current slice in the frame */
  /*44*/ uint8_t  header1_size;             /* H.264 only : size of SPS inside payload - no SPS present if value is zero */
  /*45*/ uint8_t  header2_size;             /* H.264 only : size of PPS inside payload - no PPS present if value is zero */
  /*46*/ uint8_t  reserved2[2];             /* Padding to align on 48 bytes */
  /*48*/ uint32_t advertised_size;          /* Size of frames announced as advertised frames */
  /*52*/ uint8_t  reserved3[12];            /* Padding to align on 64 bytes */
} _ATTRIBUTE_PACKED_ PaVE_t;


typedef struct {
    
    
    uint32_t    ctrl_state;             /*!< Flying state (landed, flying, hovering, etc.) defined in CTRL_STATES enum. */
    uint32_t    baterry; /*!< battery voltage filtered (mV) */

    float   theta;                  /*!< UAV's pitch in milli-degrees */
    float   phi;                    /*!< UAV's roll  in milli-degrees */
    float   psi;                    /*!< UAV's yaw   in milli-degrees */

    int32_t     altitude;               /*!< UAV's altitude in centimeters */
    int32_t   pression;

    vector31_t   v;                     /*!< UAV's estimated linear velocity */
  

    vector31_t   phys_accs;
    vector31_t   phys_gyros;

    float wind_speed;			// estimated wind speed [m/s]
    float wind_angle;	


    uint8_t     motor[4];
    
    uint32_t link_quality;
    
    double latitude;
    double longitude;
    double elevation;
    uint32_t gps_state;
    uint32_t  nbsat; //Number of acquired satellites;

} _ATTRIBUTE_PACKED_  fligth_data_t;

typedef struct {
    
    char    signature[7];
    fligth_data_t fligth_data;
    // navdata_demo_t* demo;
    PaVE_t pave;
    


} _ATTRIBUTE_PACKED_ ardata_t;

int main() {
  
    printf("%lu", sizeof(ardata_t));
    printf("%lu", sizeof(PaVE_t));
    printf("%lu", sizeof(fligth_data_t));
    return 0;
}
  