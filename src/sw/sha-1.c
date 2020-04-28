/***************************************************************************
****************************************************************************
* Filename        : sha-1.cpp
* Author          : Jishnu Murali Thampan
* Description     : Implementation of SHA-1 Alogrithm
* 		              Note: This version does not support message sizes > 512 bytes
****************************************************************************/

#include <stdio.h>
#include "sha-1.h"

#undef DEBUG_MODE /**< @brief Defining this would enable the debug prints */

#define INPUT_MESSAGE_SIZE                                                     \
  (512) /**< @brief Represents the size of the Input Message */
#define MESSAGE_SIZE                                                           \
  (64) /**< @brief Represents the 8bit array size needed to represent the      \
          input: 512/8 = 64*/
#define PRE_PROC_MSG_SIZE                                                      \
  (16) /**< @brief Represents the array size of the pre-processed message:     \
          512(message size)/ 32(bit width)*/
#define FINAL_HASH_SIZE                                                        \
  (                                                                            \
      5) /**< @brief Represents the array size of Final Hash Message: 160/32   \
            bits = 5  */

#define NUMBER_OF_STAGES                                                       \
  (4) /**< @brief Represents the number of stages of sha-1 */
#define NUMBER_OF_ROUNDS_PER_STAGE                                             \
  (20) /**< @brief Represents the number of rounds per stage of sha-1 */
#define TOTAL_NUMBER_OF_ROUNDS                                                 \
  (NUMBER_OF_STAGES) *                                                         \
      (NUMBER_OF_ROUNDS_PER_STAGE) /**< @brief Represents total number of      \
                                      rounds in sha-1 */
#define MASK_8BIT (0xff)           /**< @brief Represents the 8bit Mask*/

/* SHA1 initialization constants */
#define H0                                                                     \
  (0x67452301) /**< @brief Represents SHA1 initialization constant H0 */
#define H1                                                                     \
  (0xEFCDAB89) /**< @brief Represents SHA1 initialization constant H1 */
#define H2                                                                     \
  (0x98BADCFE) /**< @brief Represents SHA1 initialization constant H2 */
#define H3                                                                     \
  (0x10325476) /**< @brief Represents SHA1 initialization constant H3 */
#define H4                                                                     \
  (0xC3D2E1F0) /**< @brief Represents SHA1 initialization constant H4 */

#define SHA_1_ROUND_1_CONST                                                    \
  (0x5A827999) /**< @brief Represents the constant used in SHA-1 Round-1*/
#define SHA_1_ROUND_2_CONST                                                    \
  (0x6ED9EBA1) /**< @brief Represents the constant used in SHA-1 Round-2*/
#define SHA_1_ROUND_3_CONST                                                    \
  (0x8F1BBCDC) /**< @brief Represents the constant used in SHA-1 Round-3*/
#define SHA_1_ROUND_4_CONST                                                    \
  (0xCA62C1D6) /**< @brief Represents the constant used in SHA-1 Round-4*/

#define OPERATION_ROUND_1(b, c, d)                                             \
  ((b & c) |                                                                   \
   ((~b) &                                                                     \
    d)) /**< @brief Represents the cryptographic operation in SHA-1 Round-1*/
#define OPERATION_ROUND_2(b, c, d)                                             \
  ((b ^ c ^                                                                    \
    d)) /**< @brief Represents the cryptographic operation in SHA-1 Round-2*/
#define OPERATION_ROUND_3(b, c, d)                                             \
  ((b & c) | (b & d) |                                                         \
   (c &                                                                        \
    d)) /**< @brief Represents the cryptographic operation in SHA-1 Round-3*/
#define OPERATION_ROUND_4(b, c, d)                                             \
  ((b ^ c ^                                                                    \
    d)) /**< @brief Represents the cryptographic operation in SHA-1 Round-4*/

#ifdef DEBUG_MODE
/**
 * Prints the binary equivalent of a 32 bit word by iterating over each memory
 * location
 * @param[in] word Word to be printed
 * @return void
 */
static void print_word(const uint32_t word) {
  const char *bit_rep[16] = {
          [0] = "0000",  [1] = "0001",  [2] = "0010",  [3] = "0011",
          [4] = "0100",  [5] = "0101",  [6] = "0110",  [7] = "0111",
          [8] = "1000",  [9] = "1001",  [10] = "1010", [11] = "1011",
          [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
  };
  uint8_t *start_byte = (uint8_t *)&word;

  for (int i = 0; i < 4; i++) {
    uint8_t byte = *((uint8_t *)start_byte++);
    printf("%s%s ", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
  }
  printf("\n");
}
#endif
/**
 * Calculates and returns the length of the given input data
 * @param[in] input  Data whose length is to be obtained
 * @return Length of the data
 */
static size_t getLength(const void *input) {
  size_t i = 0;
  uint8_t *data = (uint8_t *)input;
  while (*((uint8_t *)(data + i)) != 0)
    i++;

  return (i);
}
/**
 * Appends '1' to the input array
 * @param[out] input Input Array to which '1' has to be appended
 * @return void
 */
static void appendOneToInputArray(uint8_t input[]) {
  size_t free_pos = getLength(input);
  if (free_pos == INPUT_MESSAGE_SIZE) {
    printf("ERR: Input sizes >= 512 is not supported");
    return;
  }
  *(input + free_pos) = (uint8_t)(0x8 << 4);
}
/**
 * Converts 8 bit message array to fixed blocks of 32 bit width
 * @param[in]  message      Message to be converted
 * @param[out] fixed_blocks Fixed blocks of 32 bit width
 * @return void
 */
static void convert_message_to_fixed_blocks(const uint8_t message[],
                                            uint32_t fixed_blocks[]) {
  /* Convert the std::string (byte buffer) to a uint32_t array (MSB) */
  for (size_t i = 0, j = 0; i < PRE_PROC_MSG_SIZE; i++, j = 4 * i) {
    fixed_blocks[i] =
        (message[j + 3] & MASK_8BIT) | (message[j + 2] & MASK_8BIT) << 8 |
        (message[j + 1] & MASK_8BIT) << 16 | (message[j + 0] & MASK_8BIT) << 24;

#ifdef DEBUG_MODE
    print_word(fixed_blocks[i]);
#endif
  }
}
/**
 * Fills the input array
 * @param[in]  data  Data to be filled
 * @param[out] input Destination array
 * @return void
 */
static void fill_input_array(const char *data, uint8_t input[]) {
  /* Store the message in its ASCII Equivalent */
  for (size_t i = 0; i < getLength(data); i++) {
    input[i] = data[i];
  }
}
/**
 * Gets the input string either hardcoded or from the user
 * @param[in] input Input array to store the user input
 * @return void
 */
static void getInputString(uint8_t input[]) {
  const char *data = "abc";
  printf("Input: Message: %s\n", data);
  fill_input_array(data, input);
}
/**
 * Rotates the given data by the number of bits specified
 * @param[in] data          input data
 * @param[in] numberOfBits  Number of Bits to be rotated
 * @return shifted value
 */
static inline uint32_t rotate_left(const uint32_t data,
                                   const size_t numberOfBits) {
  uint32_t shifted_value =
      (data << numberOfBits) | (data >> (32 - numberOfBits));
  return shifted_value;
}
/**
 * Converts the fixed blocks to chunks of data
 * @param[in]  fixed_blocks Fixed blocks
 * @param[out] chunk        Data Chunk
 * @return None
 */
static void convert_fixed_blocks_to_chunks(const uint32_t fixed_blocks[],
                                           uint32_t chunk[]) {
  size_t i = 0; /* loop variable to loop from 0 to TOTAL_NUMBER_OF_ROUNDS */

  for (; i < PRE_PROC_MSG_SIZE;
       i++) // copy the pre processed data to create a chunk
  {
    chunk[i] = fixed_blocks[i];
  }

  for (; i < TOTAL_NUMBER_OF_ROUNDS; i++) {
    chunk[i] = rotate_left(
        (chunk[i - 3] ^ chunk[i - 8] ^ chunk[i - 14] ^ chunk[i - 16]), 1);
  }
}
/**
 * Performs the Core-functionality of SHA-1 algorithm
 * @param[in]   fixed_blocks        Fixed blocks
 * @param[out]  intermediate_hashes To store the intermediate
 *              hashes obtained after computation
 * @return void
 */
static void perform_sha1_core(const uint32_t fixed_blocks[],
                              uint32_t intermediate_hashes[]) {
  uint32_t a = H0;
  uint32_t b = H1;
  uint32_t c = H2;
  uint32_t d = H3;
  uint32_t e = H4;

  uint32_t chunk[TOTAL_NUMBER_OF_ROUNDS] = {0};

  convert_fixed_blocks_to_chunks(fixed_blocks, chunk);

  uint8_t i =
      0; /* loop variable to count from 0 to NUMBER_OF_ROUNDS_PER_STAGE * 4 */

  /* Round -1 */
  for (; i < NUMBER_OF_ROUNDS_PER_STAGE; i++) {
    uint32_t temp = rotate_left(a, 5) + OPERATION_ROUND_1(b, c, d) + e +
                    SHA_1_ROUND_1_CONST + (chunk[i]);
    e = d;
    d = c;
    c = rotate_left(b, 30);
    b = a;
    a = temp;
  }
  /* Round -2 */
  for (; i < NUMBER_OF_ROUNDS_PER_STAGE * 2; i++) {
    uint32_t temp = rotate_left(a, 5) + OPERATION_ROUND_2(b, c, d) + e +
                    SHA_1_ROUND_2_CONST + (chunk[i]);
    e = d;
    d = c;
    c = rotate_left(b, 30);
    b = a;
    a = temp;
  }
  /* Round -3 */
  for (; i < NUMBER_OF_ROUNDS_PER_STAGE * 3; i++) {
    uint32_t temp = rotate_left(a, 5) + OPERATION_ROUND_3(b, c, d) + e +
                    SHA_1_ROUND_3_CONST + (chunk[i]);
    e = d;
    d = c;
    c = rotate_left(b, 30);
    b = a;
    a = temp;
  }
  /* Round -4 */
  for (; i < NUMBER_OF_ROUNDS_PER_STAGE * 4; i++) {
    uint32_t temp = rotate_left(a, 5) + OPERATION_ROUND_4(b, c, d) + e +
                    SHA_1_ROUND_4_CONST + (chunk[i]);
    e = d;
    d = c;
    c = rotate_left(b, 30);
    b = a;
    a = temp;
  }
  /* Save the intermediate hashes */
  intermediate_hashes[0] = a;
  intermediate_hashes[1] = b;
  intermediate_hashes[2] = c;
  intermediate_hashes[3] = d;
  intermediate_hashes[4] = e;
}
/**
 * Calculates and adds the input length to the fixed blocks at the end
 * @param[in]  input        Input data
 * @param[out] fixed_blocks Fixed blocks after addition of the input length
 * @return void
 */
void addInputLength(const uint8_t input[], uint32_t fixed_blocks[]) {
  uint64_t inputSize = (getLength(input) - 1) * 8;
  fixed_blocks[PRE_PROC_MSG_SIZE - 1] = (uint32_t)(inputSize);
  fixed_blocks[PRE_PROC_MSG_SIZE - 2] = (uint32_t)(inputSize >> 32);
#ifdef DEBUG_MODE
  print_word(fixed_blocks[PRE_PROC_MSG_SIZE - 1]);
  print_word(fixed_blocks[PRE_PROC_MSG_SIZE - 2]);
#endif
}
/**
 * Prints the final hash result
 * @param[in] final_hash Final Hash
 * @return void
 */
static void print_final_hash(const uint32_t final_hash[]) {
  printf("---------------------\n");
  printf("Output: SHA-1 Final Hash:\n");
  printf("========================================\n");
  for (size_t i = 0; i < FINAL_HASH_SIZE; i++) {
    printf("%x", final_hash[i]);
  }
  printf("\n========================================\n");
}
/**
 * Accumulates the intermediate hashes
 * @param[in]  intermediate_hashes Intermediate hashes
 * @param[out] result              Accumulated result
 * @return void
 */
static void accumulate_intermediate_hashes(const uint32_t intermediate_hashes[],
                                           uint32_t result[]) {
  result[0] = intermediate_hashes[0] + H0;
  result[1] = intermediate_hashes[1] + H1;
  result[2] = intermediate_hashes[2] + H2;
  result[3] = intermediate_hashes[3] + H3;
  result[4] = intermediate_hashes[4] + H4;
}
/**
 * Performs the pre-processing stage of the sha-1 algorithm
 * @param[in]  input                 Input Message
 * @param[out] pre_processed_message Message after pre-processing
 * @return void
 */
static void pre_processing_stage(uint8_t input[],
                                 uint32_t pre_processed_message[]) {
  /* Append-1 to input array */
  appendOneToInputArray(input);

  /* Now convert the message to fixed blocks */
  convert_message_to_fixed_blocks(input, pre_processed_message);

  /* Finally, add the input length at the end of the array */
  addInputLength(input, pre_processed_message);
}
/**
 * Generates the SHA-1 Hash after a series of steps
 * 1. Get Input and store its ASCII Values
 * 2. Pre-processing stage
 * 3. SHA-1 Core
 * 4. Post-processing stage
 * 5. Prints the Generated Hash
 * @param  None
 * @return void
 */
void generate_sha1_hash(void) {
  /* Get Input and store its ASCII Values*/
  uint8_t input[INPUT_MESSAGE_SIZE] = {0};
  getInputString(input);

  /* Pre-processing stage */
  uint32_t pre_processed_message[PRE_PROC_MSG_SIZE] = {0};
  pre_processing_stage(input, pre_processed_message);

  /*SHA-1 Core */
  uint32_t intermediate_hashes[FINAL_HASH_SIZE] = {0};
  perform_sha1_core(pre_processed_message, intermediate_hashes);

  /* Post-processing stage */
  uint32_t final_hash[FINAL_HASH_SIZE] = {0};
  accumulate_intermediate_hashes(intermediate_hashes, final_hash);

  /* Print the final Hash*/
  print_final_hash(final_hash);
}
