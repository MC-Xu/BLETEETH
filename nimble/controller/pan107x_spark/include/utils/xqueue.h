/**
 *******************************************************************************
 * @FileName  : x_queue.h
 * @Author    : GaoQiu
 * @CreateDate: 2020-02-18
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 *******************************************************************************
 *
 * The information contained herein is confidential and proprietary property of
 * GaoQiu and is available under the terms of Commercial License Agreement
 * between GaoQiu and the licensee in separate contract or the terms described
 * here-in.
 *
 * This heading MUST NOT be removed from this file.
 *
 * Licensees are granted free, non-transferable use of the information in this
 * file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************
 */
#ifndef X_QUEUE_H_
#define X_QUEUE_H_

#include "defs_types.h"

/*!   Initialize a queue */
#define QUEUE_INIT(pQuene)  do{(pQuene)->pHead = NULL;  (pQuene)->pTail = NULL;}while(0)

/*!   Queue type */
typedef struct{
	void *pHead;
	void *pTail;
}queue_t;


/**
 * @brief : Check if queue is empty.
 * @param : pQueue    pointer to queue.
 * @return: None.
 */
bool_t QUEUE_IsEmty(queue_t *pQueue);

/**
 * @brief: Enqueue and element to the tail of a queue.
 * @param: pQueue    pointer to queue
 * @oaram: pElem     pointer to element
 * @return: none
 */
void QUEUE_Enquene(queue_t *pQueue, void *pElem);

/**
 * @brief: Dequeue and element to the head of a queue.
 * @param: pQueue    pointer to queue
 * @return: none
 */
void *QUEUE_Dequeue(queue_t *pQueue);

/**
 * @brief: Push and element to the head of a queue.
 * @param: pQueue    pointer to queue
 * @oaram: pElem     pointer to element
 * @return: none
 */
void QUEUE_PushElement(queue_t *pQueue, void *pElem);

/**
 * @brief: Insert and element to a queue.
 * @param: pQueue    pointer to queue
 * @param: pElem     pointer to element to be inserted.
 * @param: pPrev     Pointer to previous element in the queue before element to be inserted.
 * @return: none
 */
void QUEUE_InsertElement(queue_t *pQueue, void *pElem, void *pPrev);

/**
 * @brief: Remove and element from a queue.
 * @param: pQueue    pointer to queue
 * @param: pElem     pointer to element to be inserted.
 * @param: pPrev     Pointer to previous element in the queue before element to be inserted.
 * @return: none
 */
void QUEUE_RemoveElement(queue_t *pQueue, void *pElem, void *pPrev);

#endif /* QUEUE_H_ */
