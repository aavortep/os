#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>  // для kmalloc и kfree

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petrova");

#define IRQ_NO 1

static struct workqueue_struct *my_wq;

typedef struct
{
  struct work_struct work;  // структура, описывающая функцию, которую надо запланировать
  int work_num; // номер работы
} my_work_t;

// Структуры, описывающие две работы (обработчики нижней половины прерывания в очереди работ)
static my_work_t *work_1;
static my_work_t *work_2;

static int counter = 0;

struct mutex my_mutex;

void bottom_handler(struct work_struct *work)
{
  mutex_lock(&my_mutex);
  printk(KERN_INFO "WQ: mutex locked\n");

  my_work_t *my_work = (my_work_t *)work;

  //pending=1 => в ожидании, 0 иначе
  printk(KERN_INFO "WQ: work_%d got exclusive access (work_pending = %d);\n", my_work->work_num, work_pending(&(my_work->work)));

  // другая работа заблокирована в это время?
  if (my_work->work_num == 1) //
    printk(KERN_INFO "WQ: work_2 - work_pending = %d\n", work_pending(&(work_2->work)));
  else
    printk(KERN_INFO "WQ: work_1 - work_pending = %d\n", work_pending(&(work_1->work)));

  counter++;
  printk(KERN_INFO "WQ: work_%d increased shared counter. Counter = %d\n", my_work->work_num, counter);

  mutex_unlock(&my_mutex);
  printk(KERN_INFO "WQ: mutex unlocked");
}

// TOP HALF
irqreturn_t irq_handler(int irq_num, void *dev_id)
{
  if (irq_num == IRQ_NO)
  {
    if (work_1)
      // помещение работы в очередь на выполнение
      queue_work(my_wq, (struct work_struct *)work_1);
    if (work_2)
      queue_work(my_wq, (struct work_struct *)work_2);

    return IRQ_HANDLED; // прерывание обработано
  }
  return IRQ_NONE; // прерывание не обработано
}

static int __init my_module_init(void)
{
  printk(KERN_INFO "WQ: module loaded");
	
  //Драйверы регистрируют обработчик аппаратного прерывания и разрешают определенную линию irq
  if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "my_interrupt", (void *)(irq_handler)))
  {
    printk(KERN_ERR "WQ: request_irq error");
    return -ENOMEM;
  }
  printk(KERN_INFO "WQ: irq handler registered");

  work_1 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
  work_2 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);

  // инициализация структуры work
  if (work_1)
  {
    INIT_WORK((struct work_struct *)work_1, bottom_handler);
    work_1->work_num = 1;
  }
  else
  {
    printk(KERN_ERR "WQ: kmalloc error");
    return -ENOMEM;
  }

  if (work_2)
  {
    INIT_WORK((struct work_struct *)work_2, bottom_handler);
    work_2->work_num = 2;
  }
  else
  {
    printk(KERN_ERR "WQ: kmalloc error");
  }

  // создание очереди работ
  // #define create_workqueue(name)	alloc_workqueue("%s", __WQ_LEGACY | WQ_MEM_RECLAIM, 1, (name))
  my_wq = create_workqueue("my_interrupt_wq");
  if (!my_wq)
  {
    free_irq(IRQ_NO, irq_handler);
    printk(KERN_ERR "WQ: workqueue wasn't created");
    return -ENOMEM;
  }
  printk(KERN_INFO "WQ: workqueue created successfully");
 // После успешной инициализации состояние мьютекса становится инициализированным и разблокированным.
  mutex_init(&my_mutex);

  return 0;
}

static void __exit my_module_exit(void)
{
  // принудительное завершение всех работ в очереди
  flush_workqueue(my_wq);
  // удаление очереди работ
  destroy_workqueue(my_wq);
  mutex_destroy(&my_mutex);
  // Освобождение линии прерывания
  free_irq(IRQ_NO, irq_handler);

  if (work_1)
    kfree(work_1);

  if (work_2)
    kfree(work_2);

  printk(KERN_INFO "WQ: module unloaded");
}

module_init(my_module_init);
module_exit(my_module_exit);