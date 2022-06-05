#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Petrova");

#define IRQ_NO 1  // прерывание от клавиатуры

char my_tasklet_data[] = "some tasklet data";

struct tasklet_struct my_tasklet;

// Обработчик тасклета
void tasklet_handler(unsigned long data)
{
	// state - состояние тасклета
	// count - счетчик ссылок
	// data - аргумент ф-ии обработчика тасклета
	printk(KERN_INFO "TASKLET: run tasklet");
    printk(KERN_INFO "TASKLET: state: %ld, count: %d, data: %s\n", my_tasklet.state, my_tasklet.count, my_tasklet.data);
}

// Обработчик прерывания
irqreturn_t irq_handler(int irq, void *dev)
{
  if(irq == IRQ_NO)
  {
	printk(KERN_INFO "TASKLET: state: %ld, count: %d, data: %s\n", my_tasklet.state, my_tasklet.count, my_tasklet.data);
	// планирование на выполнение
    tasklet_schedule(&my_tasklet);
    printk(KERN_INFO "TASKLET: tasklet scheduled successfully\n");
	printk(KERN_INFO "TASKLET: state: %ld, count: %d, data: %s\n", my_tasklet.state, my_tasklet.count, my_tasklet.data);

    // прерывание обработано
    return IRQ_HANDLED;
  }
  else
    // прерывание не обработано
    return IRQ_NONE;
}

static int __init my_module_init(void)
{
  printk(KERN_INFO "TASKLET: module loaded\n");

  //Драйверы регистрируют обработчик аппаратного прерывания и разрешают определенную линию irq
  int ret = request_irq(IRQ_NO, (irq_handler_t)irq_handler, IRQF_SHARED, "my_interrupt", (void *)(irq_handler));

  if (ret != 0)
  {
    printk(KERN_ERR "TASKLET: irq handler wasn't registered");
    return ret;
  }

  // инициализирует структуру tasklet_struct в соответствие с данными, предоставленными пользователем
  tasklet_init(&my_tasklet, tasklet_handler, (unsigned long)&my_tasklet_data);
  printk(KERN_INFO "TASKLET: irq handler registered successfully");

  return ret;
}

static void __exit my_module_exit(void)
{
  // ждет завершения тасклета и удаляет тасклет из очереди на выполнение только в контексте процессора
  tasklet_kill(&my_tasklet);
  // Освобождение линии irq от указанного обработчика
  free_irq(IRQ_NO, (void *)(irq_handler));

  printk(KERN_INFO "TASKLET: module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);